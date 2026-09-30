# UHE Repo Audit — Findings Summary

Scope caveat: no `cmake`/`ninja`/compiler in this container and vendored submodules are unpopulated,
so build & test health was assessed **statically only** — no configure, build, or `ctest` was run.
No source files were modified during this audit.

## Critical

1. **All uploads after the first are broken.** `VulkanDevice::ImmediateSubmit` resets
   `m_UploadCommandPool` (`UHE/src/Platform/Vulkan/VulkanDevice.cpp:695`) but `InitVulkan` never
   recreates it. Every `CreateTexture`, `GenerateMipmaps`, `UpdateTexture`, `ReadPixel` after the
   first dereferences a null pool.
2. **`PushConstants(AllGraphics, …)` pushes nothing.** The `AllGraphics` branch sits inside
   `if (m_CurrentPipelineLayout)`, making it unreachable
   (`UHE/src/Platform/Vulkan/VulkanCommandBuffer.cpp:414-431`) → `stageFlags = 0` on every engine
   draw (`UHE/src/UHE/Renderer/Renderer2D.cpp:333`, `UHE/src/UHE/Renderer3D/Renderer3D.cpp:200`).
3. **`UpdateBuffer` drops the destination offset.** `offset` is never forwarded to
   `VulkanBuffer::UploadData`, which always memcpys at the mapped base
   (`VulkanCommandBuffer.cpp:439-443`). `Renderer3D.cpp:250` uploads bone matrices with an offset, so
   all skinned models render with model 0's pose.
4. **Render graph discards the whole frame** when a declared pass handle goes unused
   (`RenderGraph/VulkanRenderGraphCompiler.cpp:180-230`): dropped passes are never seeded in `ready`,
   so a spurious `PassCycle` returns an empty graph. Fixing it naively exposes an OOB read —
   `compiled.dependencies` can contain `kInvalidIndex` (`Compiler.cpp:271-318` →
   `Executor.cpp:372`). Fix both together.
5. **A frame with no import compiles to zero passes** (`Compiler.cpp:232-288`), leaving the
   swapchain acquired with no `→Present` transition (the recovery path only runs when
   `m_FrameGraphFailed` is set, `VulkanDevice.cpp:438-450`). Exit barriers can also be grafted onto an
   unrelated pass (`Compiler.cpp:473-476`).
6. **Entity UUIDs are never assigned** (`UHE/src/UHE/Scene/Scene.cpp:248`) — every entity is ID 0;
   `SceneSerializer.cpp:17` asserts on it, so scene save is broken.
7. **Unhandled exceptions in job workers** → `std::terminate` and a permanently stuck
   `m_JobsInFlight` (`Jobsystem/Jobsystem.cpp:179-193`). `Execute()` ignores shutdown state
   (`Jobsystem.cpp:50-71`), so work queued after `ShutDown()` wedges `TaskGraph::Execute` forever
   (`Jobsystem/Taskgraph.cpp:53`).
8. **`VulkanBuffer::init` uses an uninitialized `VkBuffer`** with an assert-only failure check that
   compiles out under `NDEBUG` (`VulkanBuffer.cpp:372-376`) — garbage handle + leaked VMA allocation.

## High

- glTF attribute writes unbounded: `NORMAL`/`TEXCOORD_0`/`JOINTS_0`/`WEIGHTS_0` indexed against a
  POSITION-sized array with no check → heap OOB on malformed assets (`Renderer3D/LoadModel.cpp:224-281`);
  `asset.textures[]`/`images[]` unchecked; bufferView offsets unbounded.
- Semaphores destroyed while an in-flight `vkQueuePresentKHR` may still reference them on every
  resize — `vkDeviceWaitIdle` does not wait on the presentation engine (`VulkanDevice.cpp:199-218`).
- Bindless descriptor slots returned to the free list without nulling the descriptor
  (`VulkanDescriptorManager.cpp:236,273`) → dangling `ImageView` sampled by in-flight frames.
- `ScriptableEntity::OnUpdate` runs inside `registry.view<...>().each()` (`Scene.cpp:425`) — a
  script destroying its own entity is UAF. `DestroyEntity` never destroys
  `NativeScriptComponent::Instance` (`Scene.cpp:256` vs `Components.h:107-123`); `InstantiateScript`
  is called without a null check (`Scene.cpp:282`).
- `PhysicsSystem3D::Update`/`GetBodyInterface` deref a null system before `OnRuntimeStart` or after
  `OnRuntimeStop` (`PhysicsSystem3D.cpp:198-207`); `hardware_concurrency() - 1` underflows
  (`PhysicsSystem3D.cpp:164`); stale Jolt body IDs survive stop→start (`Scene.cpp:651`).
- Depth barriers cover only `LATE_FRAGMENT_TESTS` and omit `eDepthStencilAttachmentRead` for
  `LoadOp::Load` (`Compiler.cpp:36-45`).
- MSAA `resolve` declared in `RGColorAttachment` (`VulkanRenderGraphTypes.h:137`) and silently
  ignored by the executor (`VulkanRenderGraphExecutor.cpp:64-128`) — silently unresolved images.
- `VulkanRenderPass::Init` declares more attachments than it fills, reinterpret-casts
  `SubpassDesc[]` as `vk::SubpassDescription[]`, and passes `dependencyCount > 0` with null
  `pDependencies` (`VulkanRenderPass.cpp:16-36`).
- Editor: Play mode never calls `SetContext` on the hierarchy panel (`Editor.cpp:126-143`) → edits
  mutate the wrong registry; unbalanced `PopItemWidth` (`SceneHierachyPanel.cpp:600`); EditorCamera
  Alt anchor never re-based → one-frame wild spin (`EditorCamera.cpp:60-72`).
- Legacy render pass destroys its framebuffer while the pass is still open
  (`VulkanCommandBuffer.cpp:226-245`).
- `SceneCamera::SetOrthographic` sets `ProjectionType::Perspective`;
  `SetOrthoGraphicNearClip` returns before `RecalculateProjection()` (`SceneCamera.cpp:12`, `.h:33`).
- `colorAttachmentCount` unbounded against an 8-element array (`RHI/RHITypes.h:352`,
  `VulkanGraphicPipeline.cpp:114-151`, `VulkanPipelineState.cpp:59`).
- `ReadPixel` hardcodes `oldLayout = eShaderReadOnlyOptimal` and `eColor` aspect for an arbitrary
  texture (`VulkanDevice.cpp:727-745`).

## Medium

- Fixed storage buffers (1024 lights, 4096 bone matrices) with no cap → assert-only overflow
  (`Renderer3D.cpp:126,132-137,250-256`, `LightSystem.cpp:9-53`).
- Animator: empty-track `.back()` UB, `fmod(x, 0)` → NaN duration, unbounded bone recursion
  (`Animator.cpp:141-184,57-60,132-138`).
- `ReadPixel` per frame allocates a VMA buffer and waits `UINT64_MAX` (`Editor.cpp:499`,
  `AimLabLayer.cpp:225`); per-frame vector copies and `glm::inverse` (`Renderer3D.cpp:161,173`).
- Content browser: `directory_iterator` without `error_code` throws into the frame loop; path
  mutated while iterated; texture cache never evicted
  (`UHE_EDITOR/src/Panel/ContentBrowserPanel.cpp:18,88,100,107`).
- Depth-only `Framebuffer::Invalidate` leaks the depth attachment (`Renderer/Framebuffer.cpp:56-65`).
- Window resize uses window size while swapchain uses framebuffer size; no focus callback → cursor
  stuck disabled after Alt-Tab (`Platform/Windows/WindowsWindow.cpp:77-85,173-176`).
- Looping sounds never reclaimed from `s_ActiveSounds` (`Audio/MiniAudioBackend.cpp:58-92`).
- `LayerStack::PopLayer` can drive `m_LayerInsertIndex` negative → invalid iterator
  (`Core/LayerStack.cpp:39-47`).
- `UHE_ASSERT` expands to a bare brace block, dangling-`else` hazard (`Core/Core.h:31-48`).
- Degrees/radians mixing between Box2D and `TransformComponent` (`Scene.cpp:457,481`,
  `Components.h:24`).
- `SpriteAnimation` does not validate `FrameCount` against the sheet → negative UVs
  (`Animation2D/SpriteAnimation.cpp:42-62`).
- Scene serializer has no try/catch around YAML conversions (`SceneSerializer.cpp:283-511`).
- Sandbox depth texture does not follow window resize (`sandbox/src/ModelTestLayer.cpp:28-87`).
- `Renderer2D::Shutdown` destroys GPU objects into a deletion queue flushed only in `Begin()`, so
  everything leaks at shutdown (`Renderer/Renderer2D.cpp:251-271`).
- Camera aspect `width/height` with no zero guard in three places (`SceneCamera.cpp:22-26`,
  `EditorCamera.cpp:24-28`, `OrthograpicCameraContoroller.cpp:79-83`).

## Consistency, docs, build

- Four docs claim **"no in-tree unit tests"** (`docs/architecture/ci-cd.md:3-5`,
  `docs/README.md:30,43`, `IDEA.md:63`) — false: `enable_testing()` at `CMakeLists.txt:200` and a
  CTest `rendergraph` test at `tests/rendergraph/CMakeLists.txt:79`.
- Three docs call the RenderGraph a **stub**
  (`docs/backlog/renderer.md:47-50`, `docs/README.md:38`,
  `docs/architecture/framegraph-and-rendergraph.md:3,9`) — ~1700 implemented lines with a harness.
- Phantom paths cited by docs: `RenderGraphVulkan.{h,cpp}`, `docs/architecture/rendergraph.md`
  (cited by the test harness itself, `tests/rendergraph/CMakeLists.txt:1`), numbered `§N` sections
  that do not exist, `ci/setup-*.sh` and a `ci` preset. One archived doc still self-declares
  `Status: DESIGN`; `ROADMAP.md` `file:line` refs are stale and list already-fixed items as TODO.
- Language level declared four ways: C++20 (`readme.md:75`, `.clang-format:4`) vs C++23
  (`CMakeLists.txt:4`, `CONTRIBUTING.md:7`) — and **no C++23-only construct is used** in the tree.
- `UHE/vendor/CMakeLists.txt` is dead and broken (empty `VEGA_VENDOR_DIR`, colliding `ImGui`
  target). Stale premake path survives: `script/WinGenProject.bat` + `vendor/premake5/` with **no
  `premake5.lua`**, plus two 0-byte `ProjectUtils.cmake`.
- **No workflow runs `ctest`**, and `CMakePresets.json` has no `testPresets` — the one existing
  test has never run in automation.
- Two workflows patch `UHE/vendor/imgui/CMakeLists.txt`, which the build never adds (no-op), and
  write a stale `VEGA` output path (`build-linux.yml:44-68`, `test-renderer.yml:46-70`).
- `option(VG_PROFILE)` but consumers test `UHE_PROFILE` → Tracy flags dead
  (`CMakeLists.txt:26` vs `UHE/CMakeLists.txt:85`, `sandbox/CMakeLists.txt:29`).
- `vendor/slang/include` include path points at a nonexistent dir (`UHE/CMakeLists.txt:58`,
  `tests/rendergraph/CMakeLists.txt:46` vs `script/Setup.py:10,47`).
- RPATH configured for a SHARED lib with **zero `install()` rules** and no SOVERSION
  (`CMakeLists.txt:14-16`).
- App CMake globs lack `CONFIGURE_DEPENDS` (`sandbox/`, `UHEGAME/`, `UHE_EDITOR/`).
- Sandbox does not copy `assets/`; its model load failure is only logged, so the ASan/TSan smoke
  passes with `[SMOKE-READY]` and **no model loaded**; `test-renderer.yml:88-99` never greps
  sanitizer output (timeout 124 = pass).
- `.gitattributes`: LFS effectively off — only `filter=lfs` rule is `*.dll` (no `.dll` tracked),
  while ~70 `.ttf`, `.wav`, `.png`, `.glb`, `.fbx`, `.exe` are committed raw; lines 72-77 are
  `.gitignore`-style entries Git ignores there.
- `.clang-tidy`: all `CheckOptions` are `readability-*` but `readability-*` is not in `Checks` →
  the whole naming policy is inert. `make format` is GNU-only and omits `UHEGAME`, `sandbox`, `tests`.
- `CLA.md` retains `[OWNER LEGAL NAME]` / `[JURISDICTION]` yet CONTRIBUTING makes CLA a merge
  precondition, with no PR template and no CLA check. Bug form lacks version/GPU/driver/Vulkan
  fields — the one variable that matters for a GFX9-floor engine.
- Only `sanitizers.yml` sets `permissions:`/`concurrency:`; no actions pinned by SHA; no
  Dependabot; branch triggers `[main, master]`.
- Duplicates/debt: `sandbox/assets/` vs typo'd `sandbox/assests/`; `imgui.ini` tracked despite
  `.gitignore`; `vendor/premake5/premake5.exe` tracked despite `*.exe` ignore; unroadmapped typos
  (`AssestsManager`, `OrthograpicCameraContoroller`, `PlatfromUtils`, `RHICommadBuffer`,
  `SceneHierachy`, "will deside soon").
- Two incompatible `Renderer2D` classes in-tree: the `UHE/src/UHE/Renderer2D/` copy is excluded
  (`UHE/CMakeLists.txt:12`) but its header stays on the PUBLIC include path. Dead `Glad`
  (never linked), empty `UHE/src/UHE/Test/`, empty `Utils/ObjectPool.h`.

## Test coverage

Only `tests/rendergraph` (508-line `main.cpp`, hand-rolled `CHECK`, no UHE link, no GPU).
**Testable today with no GPU but untested:** JobSystem/TaskGraph (compiled into `rg_test` but never
exercised; the doc claiming it is "tested" commits no test), Core/Log/LayerStack/Events, Math,
ObjectPool, UUID uniqueness, `VfsSystem` path resolution (what CI's smoke depends on),
SceneSerializer round-trip, ECS, Jolt/Box2D stepping, fastgltf parsing, `VulkanTypes.cpp`
format/layout mapping, `VulkanExtensionCheck` capability strings, DeletionQueue, SlangCompiler
(CPU-only).
**GPU-required and untested:** device/swapchain/pipelines/descriptors, all three renderers, SSAO,
ImGuiLayer, editor panels, `AimLabLayer` — covered only by a Lavapipe smoke that neither greps
sanitizer output nor verifies any asset loaded.
The harness hand-duplicates UHE's include list and recompiles the same six RenderGraph files in a
different macro configuration.

## Suggested fix order

1. Render graph #4 + #5 together (memory safety), then #1/#2/#3.
2. #6/#7/#8 (UUID assignment, job-system exception + shutdown handling, buffer init).
3. glTF bounds, ECS script lifetime, physics null guards.
4. Descriptor and semaphore lifetime on resize/teardown.
5. Add `ctest` to CI; correct the four "no tests" docs and three "stub" docs.
6. Repo hygiene: LFS rules, CLA + PR template, delete the dead premake/vendor CMake path, CI
   permissions/pinning.
