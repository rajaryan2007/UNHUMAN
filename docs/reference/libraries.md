# Third-party libraries

Reference menu for `unhuman`. Nothing here is required; it is a shortlist to
consult when a subsystem needs a library.

Already vendored: `entt`, `fastgltf`, `GLFW`, `glm`, `imgui`, `imGuizmo`, `jolt`,
`box2d`, `miniaudio`, `msdf-atlas-gen`, `spdlog`, `stb_image`, `tracy`, `volk`,
`VulkanMemoryAllocator`, `yaml-cpp`, `Glad`. Slang is fetched by
`script/Setup.py`.

## Rendering and GPU

- RenderDoc (in-application API) for frame capture and debugging.
- meshoptimizer for vertex-cache optimization, meshlets and LOD. This is the key
  library for GPU-driven rendering.
- SPIRV-Reflect for shader reflection (bindings, push constants) to drive
  pipelines and descriptors.
- SPIRV-Cross to cross-compile shaders away from Vulkan if that ever matters.
- SPIRV-Tools, SPIRV-Headers and Vulkan-Headers for tooling.
- Vulkan Validation Layers, already in the SDK, should be used strictly in CI.
- vk-bootstrap is an optional instance/device init helper; device setup is
  currently hand-rolled.

## Texture and mesh compression

- Basis Universal / libktx for KTX2 and GPU-compressed textures that transcode to
  BC or ASTC at load.
- meshoptimizer / gltfpack for mesh and texture packing.
- Draco for compressed glTF meshes, and `EXT_meshopt_compression`.
- astcenc, OpenImageIO, OpenEXR, libjpeg-turbo and libwebp are optional,
  depending on which formats you import. `stb_image` is fine for development.

## Animation

- ozz-animation (Google) provides the skeletal runtime: sampling, blending, IK,
  additive layers and LOD. Its runtime is format-agnostic and loads its own
  binary archives; only the offline `gltf2ozz` tool touches glTF, so it can be fed
  from a custom cooked format.
- ACL (Animation Compression Library) for high-ratio animation compression.
- OpenSubdiv for subdivision surfaces, Alembic for baked geometry interchange,
  and OpenUSD for scene interchange if you ever need film-pipeline data.

## Audio

- miniaudio is vendored and sufficient.
- SoLoud is a simpler high-level option with built-in 3D and filters.
- FMOD and Wwise are commercial, shipping-grade choices.
- libsndfile, libvorbis and opus add codecs; Steam Audio adds HRTF spatial audio.

## Physics and navigation

- Jolt (3D) and Box2D (2D) are vendored.
- Recast and Detour, with Detour Crowd, for navmesh generation, pathfinding and
  agents.
- PhysX or Bullet only if soft bodies, cloth or GPU particles become necessary.

## ECS

- entt is vendored.
- flecs is an alternative with hierarchies, relationships, systems, pipelines and
  reflection built in. Worth evaluating if entt's lack of those becomes painful.

## Serialization and data formats

- zstd, lz4 and brotli for compression; see the compression notes below.
- FlatBuffers or Cap'n Proto for a zero-copy runtime asset format, or cereal and
  bitsery for plain C++ serialization.
- nlohmann/json, yyjson or simdjson, and toml++ for non-binary formats.
- yaml-cpp is vendored for scene serialization.

## Scripting

- sol2 with Lua is the chosen scripting path; see
  `docs/architecture/scripting.md`.
- AngelScript, Wren, QuickJS and ChaiScript are alternatives if the choice is
  ever revisited.
- CoreCLR or Mono if C# scripting is wanted, at the cost of a large runtime.
- pybind11 for Python tooling and automation.

## UI and editor

- Dear ImGui and ImGuizmo are vendored for the editor.
- ImPlot for graphs, and ImNodes or imgui-node-editor for shader and
  state-machine graphs.
- RmlUi for runtime game UI, or Noesis commercially.
- msdf-atlas-gen for text rendering.

## Fonts and text

- FreeType is installed in CI; confirm whether it is actually used. HarfBuzz for
  shaping and ICU for Unicode, if needed.

## Concurrency and jobs

- The custom Jobsystem and TaskGraph are the base.
- moodycamel::ConcurrentQueue as a lock-free MPMC queue to replace the current
  mutex plus deque.
- Taskflow, enkiTS and oneTBB are references or alternatives.

## Memory, profiling and debug

- Tracy is vendored.
- mimalloc, jemalloc or tcmalloc for the general allocator.
- Remotery, Optick or MicroProfile as lighter profilers.
- Crashpad, Breakpad or Sentry-native for crash reporting.

## Testing and quality

- doctest or Catch2 for unit tests, and Google Benchmark for microbenchmarks.
- rapidcheck for property-based tests and libFuzzer for parser fuzzing.
- clang-tidy, clang-format, include-what-you-use, cppcheck and CodeQL for static
  checks. ASan, UBSan, TSan and MSan wiring already exists in CMake.

## Build and tooling

- CMake and Ninja are in use.
- vcpkg or Conan for dependency management; ccache or sccache for caching.
- actionlint and Dependabot for the CI side.

## Networking and video

- GameNetworkingSockets (Valve) for reliable UDP with encryption; ENet, yojimbo,
  RakNet or kcp as alternatives, and GGPO or GGRS for rollback.
- FFmpeg, or pl_mpeg for a tiny MPEG1 decoder, if video playback is needed.

## Hot reload

- Lua via sol2 for script reload; a small file watcher covers shaders and
  textures. live++ or JetBrains ReSharper C++ are commercial options on Windows.

## ASWF

The Academy Software Foundation hosts the film and VFX stack. Most of it targets
offline rendering and studio pipelines rather than real-time games, so adopt
individual projects only when a concrete need appears.

| Project | Relevance |
|---|---|
| Imath | Half and HDR math types, EXR interoperability. Useful, small. |
| OpenEXR | HDR environment maps, lightmaps, IBL sources, render output. |
| OpenImageIO | Batch image conversion and comparison; suits a texture cook step, not runtime. |
| OpenColorIO | Color management and ACES pipelines. Heavy for a game engine. |
| OpenSubdiv | Catmull-Clark subdivision with GPU tessellation backends. |
| OpenVDB | Volumetric fog, clouds and smoke. |
| MaterialX | Material graph interchange; pairs with glTF, USD and Blender. |
| OpenUSD | Scene interchange and composition. A large dependency. |
| OpenTimelineIO | Editorial timeline interchange; relevant only to a cutscene sequencer. |
| OpenShadingLanguage | Not needed, since the engine uses Slang. |
| OpenCue, OpenAssetIO, OpenFX, Open Review | Render-farm and studio pipeline tools; not relevant. |

Nothing in this list belongs in the runtime hot path except loading Imath and EXR
data.

## Compression

Pick per use case rather than choosing one codec for everything.

| Codec | Use |
|---|---|
| zstd | Default for asset bundles. Best speed/ratio balance, multithreaded, and supports dictionaries for many small files. |
| zlib (DEFLATE) | Only where a format requires it. glTF and PNG already pull it in; avoid it for new data. |
| lz4 | Runtime or latency-sensitive paths; fastest decode. |
| brotli | Web or network delivery; best ratio for text. |
| LZMA/xz | Offline cook when size is critical. Slow, not for runtime. |
| blosc2 | Typed array blocks, as used by OpenVDB. |
| BasisLZ / KTX2 | Textures, as part of the KTX2 path. Not a general codec. |

In short: zstd for the asset bundle, zlib for format compatibility, lz4 only if a
decode-time need is measured, and Basis/KTX2 for textures.

## OpenTimelineIO

OTIO is an interchange format and library for editorial timelines: tracks, clips,
gaps and transitions. It comes from the film editorial world and is used to
import and export cuts to and from NLEs such as Premiere and Resolve. It is not a
runtime animation system and has no skinning, blending or playback.

Use OTIO only for a cutscene or editorial sequencer that needs timeline
interchange. For gameplay animation use ozz-animation with a state machine, and
for DCC scene interchange use USD, Alembic or glTF.

## Shortlist

Given the renderer and R&D direction, these are the libraries most worth adding
first:

1. RenderDoc in-application API for debugging the render graph.
2. meshoptimizer for GPU-driven rendering, vertex cache and LOD.
3. SPIRV-Reflect for shader reflection.
4. ozz-animation for animation systems.
5. Basis Universal / libktx for KTX2 textures.
6. sol2 and Lua for scripting and hot reload.
7. zstd for asset packing.
8. mimalloc for the allocator.
9. Recast and Detour for navigation.
10. moodycamel::ConcurrentQueue for the job queue.
