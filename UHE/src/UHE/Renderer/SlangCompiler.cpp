#include "SlangCompiler.h"
#include "uhepch.h"

#include <filesystem>
#include <fstream>
#include <glad/glad.h>
#include <slang-com-helper.h>
#include <slang-com-ptr.h>
#include <slang.h>

namespace UHE {

static Slang::ComPtr<slang::IGlobalSession> s_GlobalSession;
static Slang::ComPtr<slang::ISession> s_Session;

static void InitSlang() {
  if (s_GlobalSession)
    return;

  slang::createGlobalSession(s_GlobalSession.writeRef());
  slang::SessionDesc sessionDesc = {};

  slang::TargetDesc targets[2] = {};

  // ---------------- SPIR-V ----------------
  targets[0].format = SLANG_SPIRV;
  targets[0].profile = s_GlobalSession->findProfile("spirv_1_5");

  // ---------------- GLSL ----------------
  targets[1].format = SLANG_GLSL;
  targets[1].profile = s_GlobalSession->findProfile("glsl_450");

  sessionDesc.targets = targets;
  sessionDesc.targetCount = 2;

  s_GlobalSession->createSession(sessionDesc, s_Session.writeRef());
}

static void StringReplaceAll(std::string& source, const std::string& from, const std::string& to) {
  if (from.empty()) return;
  size_t start_pos = 0;
  while ((start_pos = source.find(from, start_pos)) != std::string::npos) {
    source.replace(start_pos, from.length(), to);
    start_pos += to.length();
  }
}

std::unordered_map<GLenum, std::string>
SlangCompiler::CompileToGLSL(const std::string &filepath) {
  InitSlang();

  std::filesystem::path p(filepath);
  std::string baseName = (p.parent_path() / p.stem()).string();

  std::unordered_map<GLenum, std::string> result;

  std::string sourceStr;
  std::ifstream in(filepath, std::ios::in | std::ios::binary);
  if (in) {
    in.seekg(0, std::ios::end);
    sourceStr.resize(in.tellg());
    in.seekg(0, std::ios::beg);
    in.read(&sourceStr[0], sourceStr.size());
    in.close();
  } else {
    UHE_CORE_ERROR("Could not open file: {0}", filepath);
    return result;
  }

  Slang::ComPtr<slang::IBlob> diagnosticBlob;
  slang::IModule *module = s_Session->loadModuleFromSourceString(
      "ShaderModule", filepath.c_str(), sourceStr.c_str(),
      diagnosticBlob.writeRef());

  if (diagnosticBlob) {
    UHE_CORE_WARN("Slang diagnostics for {0}:\n{1}", filepath,
                 (const char *)diagnosticBlob->getBufferPointer());
  }

  if (!module) {
    UHE_CORE_ERROR("Failed to load Slang module from: {0}", filepath);
    return result;
  }

  struct EntryPointInfo {
    const char *name;
    GLenum type;
    const char *suffix;
  };

  std::vector<EntryPointInfo> entryPoints = {
      {"vertexMain", GL_VERTEX_SHADER, ".vert"},
      {"fragmentMain", GL_FRAGMENT_SHADER, ".frag"}};

  for (const auto &ep : entryPoints) {
    Slang::ComPtr<slang::IEntryPoint> entryPoint;
    module->findEntryPointByName(ep.name, entryPoint.writeRef());

    if (!entryPoint) {
      UHE_CORE_ERROR("Could not find Slang entry point '{0}' in {1}", ep.name,
                    filepath);
      continue;
    }

    slang::IComponentType *components[] = {module, entryPoint};
    Slang::ComPtr<slang::IComponentType> program;

    s_Session->createCompositeComponentType(components, 2, program.writeRef(),
                                            diagnosticBlob.writeRef());

    Slang::ComPtr<slang::IComponentType> linkedProgram;
    program->link(linkedProgram.writeRef(), diagnosticBlob.writeRef());

    if (diagnosticBlob) {
      const char *msg = (const char *)diagnosticBlob->getBufferPointer();
      if (msg && msg[0])
        UHE_CORE_WARN("Slang link diagnostics:\n{0}", msg);
    }

    // 1. SPIR-V Target (Index 0)
    Slang::ComPtr<slang::IBlob> spirvBlob;
    linkedProgram->getTargetCode(0, spirvBlob.writeRef(),
                                 diagnosticBlob.writeRef());

    if (spirvBlob) {
      std::string outPath = baseName + ep.suffix + ".spv";
      std::ofstream out(outPath, std::ios::binary);
      out.write((char *)spirvBlob->getBufferPointer(),
                spirvBlob->getBufferSize());
    }

    // 2. GLSL Target (Index 1)
    Slang::ComPtr<slang::IBlob> glslBlob;
    linkedProgram->getTargetCode(1, glslBlob.writeRef(),
                                 diagnosticBlob.writeRef());

    if (glslBlob) {
      std::string outPath = baseName + ep.suffix + ".glsl";
      std::ofstream out(outPath);
      out.write((char *)glslBlob->getBufferPointer(),
                glslBlob->getBufferSize());

      std::string code((const char *)glslBlob->getBufferPointer(),
                       glslBlob->getBufferSize());
      result[ep.type] = code;
    } else {
      UHE_CORE_ERROR("Failed to generate GLSL target code for entry point: {0}",
                    ep.name);
      if (diagnosticBlob) {
        UHE_CORE_ERROR("Error: {0}",
                      (const char *)diagnosticBlob->getBufferPointer());
      }
    }
  }

  // --- UNWRAP SLANG UNIFORM BLOCKS BACK TO LOOSE UNIFORMS FOR UHE OpenGL BACKEND ---

  //TODO : remove the this and opengl backend later and swift to toward vulkan 
  for (auto& pair : result) {
    std::string& code = pair.second;
    
    std::string badBlock = 
        "layout(binding = 0)\n"
        "layout(std140) uniform block_GlobalParams_0\n"
        "{\n"
        "    mat4x4 u_ViewProjection_0;\n"
        "    mat4x4 u_Transform_0;\n"
        "    vec4 u_Color_0;\n"
        "}globalParams_0;";
    std::string goodLoose = 
        "uniform mat4 u_ViewProjection;\n"
        "uniform mat4 u_Transform;\n"
        "uniform vec4 u_Color;";
        
    StringReplaceAll(code, badBlock, goodLoose);

    // Strip the _0 mangling from usages in the shader main()
    StringReplaceAll(code, "globalParams_0.u_ViewProjection_0", "u_ViewProjection");
    StringReplaceAll(code, "globalParams_0.u_Transform_0", "u_Transform");
    StringReplaceAll(code, "globalParams_0.u_Color_0", "u_Color");

    // Also fix the texture array rename from u_Texture_0 to u_Texture
    StringReplaceAll(code, "u_Texture_0", "u_Texture");

    // Strip `layout(binding = 1)` from sampler array so bindings are
    // controlled exclusively by the glUniform1iv call in Renderer2D::Init.
    // Some drivers cache the layout binding qualifier and intermittently
    // ignore the explicit uniform override, causing wrong-unit sampling.
    StringReplaceAll(code, "layout(binding = 1)\nuniform sampler2D", "uniform sampler2D");

    // Fix non-uniform dynamic indexing into the sampler array.
    // When fragments in the same GPU warp have different TexIndex values
    // (e.g. textured face vs solid-color face at a triangle boundary),
    // the GPU may use one fragment's index for the whole warp, sampling
    // from the wrong (potentially unbound) texture unit → black dots.
    // Wrapping with nonuniformEXT tells the GPU to handle divergent access.
    StringReplaceAll(code,
        "u_Texture[int(input_v_TexIndex_0)]",
        "u_Texture[nonuniformEXT(int(input_v_TexIndex_0))]");
  }

  // Inject the nonuniform qualifier extension into the fragment shader
  if (result.count(GL_FRAGMENT_SHADER)) {
    std::string& fragCode = result[GL_FRAGMENT_SHADER];
    std::string versionLine = "#version 450\n";
    size_t vpos = fragCode.find(versionLine);
    if (vpos != std::string::npos) {
      fragCode.insert(vpos + versionLine.size(),
        "#extension GL_EXT_nonuniform_qualifier : enable\n");
    }
  }

  // Save a combined GLSL file that UHE's OpenGL backend can read natively via
  
  std::string combinedGLSLPath = baseName + ".glsl";
  std::ofstream combinedOut(combinedGLSLPath);
  if (combinedOut) {
    if (result.count(GL_VERTEX_SHADER)) {
      combinedOut << "#type vertex\n";
      combinedOut << result[GL_VERTEX_SHADER] << "\n";
    }
    if (result.count(GL_FRAGMENT_SHADER)) {
      combinedOut << "#type fragment\n";
      combinedOut << result[GL_FRAGMENT_SHADER] << "\n";
    }
  }

  return result;
}

std::unordered_map<RHI::ShaderStage, std::vector<uint8_t>> SlangCompiler::CompileToSPIRV(const std::string& filepath) {
    std::unordered_map<RHI::ShaderStage, std::vector<uint8_t>> result;

    Slang::ComPtr<slang::IGlobalSession> slangGlobalSession;
    slang::createGlobalSession(slangGlobalSession.writeRef());

    slang::SessionDesc sessionDesc = {};
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = slangGlobalSession->findProfile("glsl_450");
    
    sessionDesc.targets = &targetDesc;
    sessionDesc.targetCount = 1;
    sessionDesc.defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR;

    Slang::ComPtr<slang::ISession> session;
    slangGlobalSession->createSession(sessionDesc, session.writeRef());

    Slang::ComPtr<slang::IBlob> diagnosticBlob;
    slang::IModule* module = session->loadModule(filepath.c_str(), diagnosticBlob.writeRef());

    if (diagnosticBlob) {
        const char* msg = (const char*)diagnosticBlob->getBufferPointer();
        if (msg && msg[0])
            UHE_CORE_WARN("Slang compiler diagnostics:\n{0}", msg);
    }

    if (!module) {
        UHE_CORE_ERROR("Failed to load slang module: {0}", filepath);
        return result;
    }

    struct EntryPointInfo {
        const char* name;
        RHI::ShaderStage stage;
    };

    EntryPointInfo entryPoints[] = {
        { "vertexMain", RHI::ShaderStage::Vertex },
        { "fragmentMain", RHI::ShaderStage::Fragment }
    };

    for (const auto& ep : entryPoints) {
        Slang::ComPtr<slang::IEntryPoint> entryPoint;
        module->findEntryPointByName(ep.name, entryPoint.writeRef());
        
        if (!entryPoint) continue;

        slang::IComponentType* components[] = { module, entryPoint };
        Slang::ComPtr<slang::IComponentType> program;
        
        session->createCompositeComponentType(components, 2, program.writeRef(), diagnosticBlob.writeRef());
        
        Slang::ComPtr<slang::IComponentType> linkedProgram;
        program->link(linkedProgram.writeRef(), diagnosticBlob.writeRef());
        
        if (diagnosticBlob) {
            const char* msg = (const char*)diagnosticBlob->getBufferPointer();
            if (msg && msg[0]) UHE_CORE_WARN("Slang link diagnostics:\n{0}", msg);
        }

        Slang::ComPtr<slang::IBlob> spirvBlob;
        linkedProgram->getTargetCode(0, spirvBlob.writeRef(), diagnosticBlob.writeRef());
        
        if (spirvBlob) {
            const uint8_t* ptr = (const uint8_t*)spirvBlob->getBufferPointer();
            size_t size = spirvBlob->getBufferSize();
            result[ep.stage] = std::vector<uint8_t>(ptr, ptr + size);
        } else {
            UHE_CORE_ERROR("Failed to generate SPIR-V code for entry point: {0}", ep.name);
            if (diagnosticBlob) {
                UHE_CORE_ERROR("Error: {0}", (const char*)diagnosticBlob->getBufferPointer());
            }
        }
    }

    return result;
}

} // namespace UHE
