
#include "nightpch.h"
#include "DebugRenderer.h"
#include "utility.h"
#include "node/INode.h"
#include "texture/ITexture.h"
#include "math/Math.h"
#include "texture/Surface.h"
#include "raycast/Raycast.h"
#include "camera/Camera.h"
#include "profiler/Profiler.h"
#include "application/Application.h"
#include "node/NodeRenderTarget.h"
#include "node/NodeWindow.h"
#include "shape_renderer/ShapeRenderer.h"

// TODO: may need to dispatch main thread callbacks before flushing renderer
// TODO: make involved nodes a stack.
// TODO: exclude text rendering of parent algorithm steps.
// TODO: make gui accessable through algo callback, we can use sliders and buttons to see better

#define NIGHT_DB_RENDERER_ALGO_FADE 0.75f
#ifdef NIGHT_ENABLE_DEBUG_RENDERER

namespace night
{
    map<vector<handle<INode>>, map<string, DebugRenderer::Algorithm>> DebugRenderer::_algoInvolvedNodes;
    vector<DebugRenderer::CIN> DebugRenderer::_algoCurrentlyInvolvedNodesStack;
    vector<handle<INode>> DebugRenderer::_guiSelectedInvolvedNodes;
    vector<pair<string, s32>>  DebugRenderer::_guiSelectedAlgorithmSteps;
    DebugRenderer::Break DebugRenderer::_guiBreak;
    string DebugRenderer::_guiSelectedAlgorithm;
     handle<ITexture> DebugRenderer::_renderTarget = nullptr;
    RenderGraph DebugRenderer::_renderGraph;
    EDebugRendererTab DebugRenderer::_guiCurrentTab = EDebugRendererTab::Scene;
    umap<type_index, function<void(void*)>> DebugRenderer::_sceneObjectDrawFunctionTable;
    umap<handle<INode>, b8> DebugRenderer::_sceneSelectedNodes;
    b8 _isViewingAlgo = false;
    CameraTransform DebugRenderer::_algoCameraTransform = {};
    string DebugRenderer::_assetsSelectedTexture = "";
    b8 DebugRenderer::_assetsTextureShowDepthBuffer = false;
    b8 DebugRenderer::_assetsShouldSaveDebugTextures = false;

    static INode dummy_node = {};

    // TODO: may want to make debug renderer into a framebuffer node.
    void DebugRenderer::init()
    {
        sref<Surface> surface(new Surface(SurfaceParams{ .width = utility::renderer().width(), .height = utility::renderer().height(), .fill_color = {0.0f, 0.0f, 0.0f, 0.0f} }));
        _renderTarget = utility::renderer().create_texture("__DEBUG_Render_Target", { .surface = surface });
        ASSERT(_renderTarget != nullptr);
        _renderTarget->render_flush_priority(10000.0f + (DB_ALGO_MAX_DEPTH + 1));
        _renderTarget->should_use_depth_peeling = false;
        _renderTarget->should_use_depth_testing = false;
        _renderTarget->should_use_blending = true;

        //_nodlessDummyNode = shandle<INode>(&dummy_node);

        //algo_involve_nodes(handle<INode>()); // base nodes are null
        //algo_involve_nodes(handle<INode>(_nodlessDummyNode));
    }

    // TODO: don't do a map lookup every step.
    void DebugRenderer::algo_push(string const& name)
    {
        if (!_algoIsActive || _algoCurrentlyInvolvedNodesStack.empty())
        {
            return;
        }

        //ASSERT(!_algoCurrentlyInvolvedNodesStack.empty());

        auto& top = _algoCurrentlyInvolvedNodesStack.back();
        ASSERT(!top.involved_nodes.empty());
        //ASSERT(!top.algorithm_stack.empty());

        top.algorithm_stack.push_back(name);

        auto& nodes = _algoInvolvedNodes[top.involved_nodes];

        auto* curr = &nodes[top.algorithm_stack[0]];

        for (s32 i = 1; i < top.algorithm_stack.size(); i++)
        {
            auto& algo_name = top.algorithm_stack[i];

            //ASSERT(!curr->render_steps.empty());
            if (curr->render_steps.empty())
            {
                curr->render_steps.push_back({});
            }

            curr = &curr->render_steps.back().sub_steps[algo_name];
        }

        //curr->render_steps.push_back({});
        curr->render_steps.clear();
    }

    void DebugRenderer::algo_increment_step()
    {
        if (!_algoIsActive || _algoCurrentlyInvolvedNodesStack.empty())
        {
            return;
        }

        auto& top = _algoCurrentlyInvolvedNodesStack.back();
        ASSERT(!top.involved_nodes.empty());
        ASSERT(!top.algorithm_stack.empty());

        auto& nodes = _algoInvolvedNodes[top.involved_nodes];

        auto* curr = &nodes[top.algorithm_stack[0]];

        for (s32 i = 1; i < top.algorithm_stack.size(); i++)
        {
            auto& algo_name = top.algorithm_stack[i];

            ASSERT(!curr->render_steps.empty());

            curr = &curr->render_steps.back().sub_steps[algo_name];
        }

        curr->render_steps.push_back({});
    }

    void DebugRenderer::algo_pop()
    {
        if (!_algoIsActive || _algoCurrentlyInvolvedNodesStack.empty())
        {
            return;
        }

        auto& top = _algoCurrentlyInvolvedNodesStack.back();
        ASSERT(!top.involved_nodes.empty());
        ASSERT(!top.algorithm_stack.empty());

        top.algorithm_stack.pop_back();
    }

    void DebugRenderer::algo_uninvolve_nodes()
    {
        ASSERT(!_algoCurrentlyInvolvedNodesStack.empty());
        _algoCurrentlyInvolvedNodesStack.pop_back();
    }

    void DebugRenderer::render()
    {
        ASSERT(_algoCurrentlyInvolvedNodesStack.empty());

        for (auto i = _algoInvolvedNodes.begin(); i != _algoInvolvedNodes.end();)
        {
            for (s32 j = 0; j < (*i).first.size(); j++)
            {
                if ((*i).first[j] == nullptr)
                {
                    i = _algoInvolvedNodes.erase(i);
                    goto CONTINUE;
                }
            }

            i++;
        CONTINUE:
            continue;
        }

        for (const auto& i : _guiSelectedInvolvedNodes)
        {
            if (i == nullptr)
            {
                _guiSelectedInvolvedNodes.clear();
                break;
            }
        }

        utility::renderer().flush();
        _renderTarget->clear(COLOR_ZERO);
        _renderGraph.clear();
        _renderGraph.append_transform(mat4(1));

        auto& gui = utility::gui();

        if (gui.begin("Debug Renderer"))
        {
            b8 paused = Application::get().is_paused();

            if (gui.button(paused ? ">" : "O"))
            {
                //paused ? Application::get().pause() : Application::get().unpause();
                Application::get()._isPaused = !Application::get()._isPaused;
            }

            gui.same_line();

            if (gui.button(">>"))
            {
                Application::get().frame_advance();
            }

            gui.same_line();

            gui.drag_real("Time Scale", &utility::window().timescale(), 0.05f, 0.0f, INFINITY);

            gui.seperator();

            if (gui.button("Scene"))
            {
                _guiCurrentTab = EDebugRendererTab::Scene;
            }

            gui.same_line();

            if (gui.button("Algorithms"))
            {
                _guiCurrentTab = EDebugRendererTab::Algorithms;
            }

            gui.same_line();

            if (gui.button("Profiler"))
            {
                _guiCurrentTab = EDebugRendererTab::Profiler;
            }

            gui.same_line();

            if (gui.button("Assets"))
            {
                _assetsSelectedTexture.clear();
                _guiCurrentTab = EDebugRendererTab::Assets;
            }

            switch (_guiCurrentTab)
            {
            case EDebugRendererTab::Scene:
            {
                scene_render();
                break;
            }

            case EDebugRendererTab::Algorithms:
            {
                algo_render();
                break;
            }

            case EDebugRendererTab::Profiler:
            {
                profiler_render();
                break;
            }

            case EDebugRendererTab::Assets:
            {
                assets_render();
                break;
            }
            }
        }

        gui.end();

        _algoCurrentlyInvolvedNodesStack.clear();
        // TODO: us dummy node because nullptr is used to clean up algo selection view.
        //algo_involve_nodes(handle<INode>());
        //algo_involve_nodes(handle<INode>(_nodlessDummyNode));
    }

    void DebugRenderer::scene_render()
    {
        // render scene tree:

        _renderGraph.current_buffer(_renderTarget, nullptr, nullptr);

        auto& gui = utility::gui();
        handle<INode> root = Application::get().root();

        if (root == nullptr)
        {
            return;
        }

        function<void(handle<INode>, handle<NodeRenderTarget>, s32)> rec1 = [&](handle<INode> maybe_window, handle<NodeRenderTarget> crt, s32 index)
            {
                b8 maybe = maybe_window->is_of_type<NodeWindow>();
                if (maybe)
                {
                    auto& is_selected = _sceneSelectedNodes[maybe_window];

                    string label = !is_selected ? "-" : "0";

                    if (maybe_window->is_signal_space())
                    {
                        label = !is_selected ? "(-)" : "(0)";
                    }

                    for (s32 i = 0; i < index; i++)
                    {
                        label += "##xx";
                    }
                    gui.button(label); // TODO: dont add function if it does not exist.
                    
                    if (gui.is_item_clicked())
                    {
                        is_selected = !is_selected;
                    }

                    gui.same_line();
                    gui.tree_node(maybe_window->name() + "(" + to_string(maybe_window->unique_id()) + ")", is_selected);

                    if (is_selected)
                    {
                        handle<NodeWindow> nw = maybe_window;

                        _renderGraph.current_buffer(_renderTarget, nullptr, nullptr); // TODO: may remove

                        _renderTarget->resize(ivec2(nw->width(), nw->height()));
                        _renderTarget->clear(COLOR_ZERO);
                        Application::get().dispatch_main_thread_callbacks();
                        //utility::renderer().update_resources();

                        // render child nodes of window node to _renderTarget
                        function<void(handle<INode>, s32)> rec2 = [&](handle<INode> not_window, s32 index)
                            {
                                ASSERT(not_window != nullptr);

                                if (not_window->is_of_type<NodeWindow>())
                                {
                                    return;
                                }

                                auto& is_selected = _sceneSelectedNodes[not_window];

                                string label = !is_selected ? "-" : "0";
                                for (s32 i = 0; i < index; i++)
                                {
                                    label += "##xx";
                                }

                                if (not_window->is_signal_space())
                                {
                                    label = !is_selected ? "(-)" : "(0)";
                                }

                                gui.button(label); // TODO: dont add function if it does not exist.
                                if (gui.is_item_clicked())
                                {
                                    is_selected = !is_selected;
                                }

                                gui.same_line();
                                gui.tree_node(not_window->name() + "(" + to_string(not_window->unique_id()) + ")", is_selected);

                                if (is_selected)
                                {
                                    auto f = _sceneObjectDrawFunctionTable.find(not_window->type_id());
                                    if (f != _sceneObjectDrawFunctionTable.end())
                                    {
                                        auto crt = not_window->current_render_target();
                                        ASSERT(crt != nullptr);
                                        Camera camera = crt->camera();

                                        _renderTarget->camera(camera);

                                        void* generic = (void*)not_window.ptr().lock().get();
                                        (*f).second(generic);

                                        utility::renderer().flush_render_graph(_renderGraph);
                                        utility::renderer().flush();
                                        _renderGraph.clear();
                                        _renderGraph.current_buffer(_renderTarget, nullptr, nullptr);
                                        _renderGraph.append_transform(mat4(1));
                                    }
                                }

                                for (s32 i = 0; i < not_window->children().size(); i++)
                                {
                                    rec2(not_window->children()[i], i);
                                }

                                gui.tree_pop();
                            };

                        for (s32 i = 0; i < maybe_window->children().size(); i++)
                        {
                            rec2(maybe_window->children()[i], i);
                        }

                        Quad<> q = Quad<>(nw->base_render_target_window_area());
                        _renderGraph.current_buffer(
                            utility::renderer().default_render_target(),
                            nullptr,
                            _renderTarget
                        );
                        _renderGraph.draw_quad(q);
                        utility::renderer().flush_render_graph(_renderGraph);
                        utility::renderer().flush();
                        _renderGraph.clear();
                        _renderGraph.append_transform(mat4(1));
                    }
                }

                for (s32 i = 0; i < maybe_window->children().size(); i++)
                {
                    auto& child = maybe_window->children()[i];
                    rec1(child, crt, i);
                }

                if (maybe)
                {
                    gui.tree_pop();
                }
            };

        rec1(root, nullptr, 0);
    }

    void DebugRenderer::algo_render()
    {
        auto& gui = utility::gui();
        b8 is_viewing_step = false;

        if (gui.button(_algoIsActive ? "Deactivate" : "Activate"))
        {
            _algoIsActive = !_algoIsActive;
        }

        gui.same_line();

        if (gui.button("Clear"))
        {
            _guiSelectedInvolvedNodes.clear();
            _guiSelectedAlgorithmSteps.clear();
            _guiBreak = {};
            _guiSelectedAlgorithm.clear();
            _algoInvolvedNodes.clear();
            _algoCurrentlyInvolvedNodesStack.clear();
            _algoCameraTransform = {};
        }

        gui.seperator();

        // display node tree, allow nodes to be selected
        for (auto i = _algoInvolvedNodes.begin(); i != _algoInvolvedNodes.end(); i++)
        {
            //if ((*i).first.empty())
            //{
            //    if (gui.selectable("Nodeless", false))
            //    {

            //    }
            //}

            string names;

            //if ((*i).first.size() == 1 && (*i).first.back() == _nodlessDummyNode)
            //{
            //    names = "Nodeless";
            //}
            //else
            {
                for (s32 j = 0; j < (*i).first.size(); j++)
                {
                    if ((*i).first[j] == nullptr)
                    {
                        continue;
                    }

                    names += (*i).first[j]->name() + "(" + to_string((*i).first[j]->unique_id()) + ")";
                    if (j < (*i).first.size() - 1)
                    {
                        names += ", ";
                    }
                }
            }

            if (names.empty())
            {
                continue;
            }

            gui.selectable(names, _guiSelectedInvolvedNodes == (*i).first);
            {
                if (gui.is_item_clicked())
                {
                    if (_guiSelectedInvolvedNodes != (*i).first)
                    {

                        _guiSelectedInvolvedNodes = (*i).first;
                    }
                    else
                    {
                        _guiSelectedInvolvedNodes.clear();
                    }

                    _guiSelectedAlgorithm.clear();
                    _guiSelectedAlgorithmSteps.clear();
                }
            }
        }

        //for (const auto& i : _guiSelectedInvolvedNodes)
        //{
        //    if (i == nullptr)
        //    {
        //        _guiSelectedInvolvedNodes.clear();
        //    }
        //}


        // drag step sliders
        if (!_guiSelectedInvolvedNodes.empty())// TODO: this wont work with Nodeless
        {
            {
                // render darkness
                _renderGraph.current_buffer(utility::renderer().default_render_target(), nullptr, nullptr);

                s32 w = utility::renderer().default_render_target()->width(); // TODO: maybe use NodeFrameBuffer
                s32 h = utility::renderer().default_render_target()->height();
                vec2 ar = { (h < w ? (real)h / (real)w : 1.0f), (w < h ? (real)w / (real)h : 1.0f) };
                vec3 scale = vec3(1.0f / ar.x, 1.0f / ar.y, 1.0f);

                _renderGraph.draw_quad(QuadParams<>{ .position = ORIGIN, .size = scale, .color = BLACK.opaqued(0.95f) }); // lower alpha
                utility::renderer().flush_render_graph(_renderGraph);
                utility::renderer().flush();
                _renderGraph.clear();
                _renderGraph.append_transform(mat4(1));
            }

            auto tree = _algoInvolvedNodes.find(_guiSelectedInvolvedNodes);

            if (tree != _algoInvolvedNodes.end())
            {
                gui.text("Algorithms: ");
                for (auto i = (*tree).second.begin(); i != (*tree).second.end(); i++)
                {
                    gui.selectable((*i).first + "##xx", _guiSelectedAlgorithm == (*i).first);
                    if (gui.is_item_clicked())
                    {
                        if (_guiSelectedAlgorithm != (*i).first)
                        {
                            _guiSelectedAlgorithm = (*i).first;
                        }
                        else
                        {
                            _guiSelectedAlgorithm.clear();
                        }

                        _guiSelectedAlgorithmSteps.clear();
                    }
                }

                if (!_guiSelectedAlgorithm.empty())
                {
                    auto f = (*tree).second.find(_guiSelectedAlgorithm);
                    if (f != (*tree).second.end())
                    {
                        gui.text("Steps: ");

                        map<string, Algorithm> dummy_map; // TODO: this is dumb.
                        dummy_map.insert({ (*f).first, (*f).second });

                        Camera camera;
                        if (!_guiSelectedInvolvedNodes.empty())
                        {
                            //ASSERT(_guiSelectedInvolvedNodes.front() != nullptr);
                            if (_guiSelectedInvolvedNodes.front() == nullptr)
                            {
                                Camera c;
                                c.type = ECameraType::Perspective;
                                c.fov = 60.0f;
                                c.translation = FORWARD * (real)10;
                                c.look_at = ORIGIN;
                                c.near_clip = NIGHT_CAMERA_DEFAULT_NEAR_CLIP;
                                c.far_clip = NIGHT_CAMERA_DEFAULT_FAR_CLIP;
                                c.up = UP;
                                camera = c;
                            }
                            else
                            {
                                auto crt = _guiSelectedInvolvedNodes.front()->current_render_target();
                                ASSERT(crt != nullptr);
                                camera = crt->camera();
                            }
                        }
                        else
                        {
                            Camera c;
                            c.type = ECameraType::Perspective;
                            c.fov = 60.0f;
                            c.translation = FORWARD * (real)10;
                            c.look_at = ORIGIN;
                            c.near_clip = NIGHT_CAMERA_DEFAULT_NEAR_CLIP;
                            c.far_clip = NIGHT_CAMERA_DEFAULT_FAR_CLIP;
                            c.up = UP;
                            camera = c;
                            //camera = utility::renderer().default_render_target()->camera();
                        }

                        camera = camera * _algoCameraTransform;

                        _renderTarget->camera(camera);
                                               
                        function<s32(map<string, Algorithm> const&, s32)> rec = [&](map<string, Algorithm> const& branches, s32 depth) -> s32
                            {
                                if (_guiSelectedAlgorithmSteps.size() <= depth)
                                {
                                    _guiSelectedAlgorithmSteps.emplace_back("", -1);
                                }

                                string& selected_name = _guiSelectedAlgorithmSteps[depth].first;
                                s32& selected_step = _guiSelectedAlgorithmSteps[depth].second;

                                string next_name = "";
                                s32 inverse_depth = 0;

                                for (auto i = branches.begin(); i != branches.end(); i++)
                                {
                                    string const& name = (*i).first;
                                    Algorithm const& algo = (*i).second;

                                    ASSERT(!name.empty());

                                    if (name == selected_name)
                                    {
                                        s32 prev = selected_step;
                                        string tabs;
                                        for (s32 i = 0; i < depth; i++)
                                        {
                                            tabs += "  ";
                                        }
                                        gui.text(tabs);
                                        gui.same_line();
                                        gui.drag_s32(" " + name, &selected_step, 0.1f, -1, (s32)algo.render_steps.size() - 1);
                                        selected_step = CLAMP(selected_step, -1, (s32)algo.render_steps.size() - 1);

                                        // clear all steps ahead of selected
                                        if (selected_step != prev && depth + 1 < _guiSelectedAlgorithmSteps.size())
                                        {
                                            for (s32 j = depth + 1; j < _guiSelectedAlgorithmSteps.size(); j++)
                                            {
                                                _guiSelectedAlgorithmSteps[j].first = "";
                                                _guiSelectedAlgorithmSteps[j].second = -1;
                                            }
                                        }

                                        if (selected_step == -1)
                                        {
                                            inverse_depth = -1;
                                            continue;
                                        }

                                        auto& render_step = algo.render_steps[selected_step];
                                        auto& callbacks = render_step.callbacks;
                                        auto& sub_steps = render_step.sub_steps;

                                        if (!sub_steps.empty())
                                        {
                                            inverse_depth = rec(sub_steps, depth + 1) + 1; // less depth, bigger number
                                        }

                                        if (!callbacks.empty())
                                        {
                                            _renderGraph.current_buffer(_renderTarget, nullptr, nullptr);

                                            _isViewingAlgo = true;
                                            for (const auto& i : callbacks)
                                            {
                                                i();
                                            }
                                            _isViewingAlgo = false;

                                            utility::renderer().flush_render_graph(_renderGraph);
                                            utility::renderer().flush();
                                            _renderGraph.clear();
                                            _renderGraph.append_transform(mat4(1));

                                            _renderGraph.current_buffer(
                                                utility::renderer().default_render_target(),
                                                nullptr,
                                                _renderTarget
                                            );

                                            Quad area;

                                            if (!_guiSelectedInvolvedNodes.empty() && _guiSelectedInvolvedNodes.front() != nullptr)
                                            {
                                                auto nw = _guiSelectedInvolvedNodes.front()->find_parent<NodeWindow>();
                                                if (nw != nullptr)
                                                {
                                                    area = Quad<>(nw->base_render_target_window_area());
                                                }
                                                else
                                                {
                                                    area = Quad(utility::renderer().default_render_target()->area());
                                                }
                                            }
                                            else
                                            {
                                                area = Quad(utility::renderer().default_render_target()->area());
                                            }

                                            Color color = WHITE;
                                            color.a /= (1 + (inverse_depth * NIGHT_DB_RENDERER_ALGO_FADE));
                                            area.vertices[0].color = color;
                                            area.vertices[1].color = color;
                                            area.vertices[2].color = color;
                                            area.vertices[3].color = color;

                                            _renderGraph.draw_quad(area);
                                            utility::renderer().flush_render_graph(_renderGraph);
                                            utility::renderer().flush();
                                            _renderGraph.clear();
                                            _renderGraph.append_transform(mat4(1));

                                            is_viewing_step = true;
#if 0
                                            ASSERT(inverse_depth + 1 < (s32)_algoRenderTargets.size());
                                            ASSERT(_algoRenderTargets[inverse_depth + 1] != nullptr);
                                            _renderGraph.current_buffer(_algoRenderTargets[inverse_depth + 1], nullptr, nullptr);

                                            _isViewingAlgo = true;
                                            for (const auto& i : callbacks)
                                            {
                                                i();
                                            }
                                            _isViewingAlgo = false;

                                            is_viewing_step = true;
#endif
                                        }
                                    }
                                    else
                                    {
                                        s32 dummy_step = -1;
                                        string tabs;
                                        for (s32 i = 0; i < depth; i++)
                                        {
                                            tabs += "  ";
                                        }
                                        gui.text(tabs);
                                        gui.same_line();
                                        gui.drag_s32(" " + name, &dummy_step, 0.1f, -1, 0);

                                        if (dummy_step == 0)
                                        {
                                            next_name = name;
                                        }
                                    }

                                    if (!next_name.empty())
                                    {
                                        selected_name = next_name;
                                        selected_step = 0;
                                    }

                                    if (selected_name.empty())
                                    {
                                        inverse_depth = -1;
                                    }
                                }

                                return inverse_depth;
                            };

                        rec(dummy_map, 0);
                    }
                }
            }
        }

        static vec3 rotation = vec3(0);
        if (!is_viewing_step)
        {
            rotation = vec3(0);
            _algoCameraTransform = {};
            return;
        }

        // handle breaking:
        _guiBreak = {};

        if (gui.button("Break")) // check if step is selected
        {
            for (s32 i = 0; i < _guiSelectedAlgorithmSteps.size(); i++)
            {
                auto& step = _guiSelectedAlgorithmSteps[i];
                if (step.second == -1)
                {
                    break;
                }

                //_guiBreak.step = step.second; // TODO: do something better
                //_guiBreak.algo_stack.push_back(step.first);
                _guiBreak.algorithm_stack.push_back(step);
            }

            _guiBreak.involved_nodes = _guiSelectedInvolvedNodes;
        }
        
        static b8 _is_controling_camera = false;
        static vec2 _locked_mouse_position = ORIGIN;

        gui.seperator();

        _wantsInput = false;

        if (gui.button("Reset Camera"))
        {
            _algoCameraTransform = {};
            rotation = vec3(0);
        }

        gui.begin_canvas("Fly", 1.0f, BLACK);

        if (utility::window().is_mouse_grabbed() && !_is_controling_camera)
        {
            // do not enter fly mode if mouse is already grabbed.
            gui.end_canvas();
            return;
        }
        if (gui.is_item_clicked())
        {
            if (!_is_controling_camera)
            {
                utility::window().grab_mouse();
                _is_controling_camera = true;
                _wantsInput = true;
                _locked_mouse_position = utility::window().mouse();
            }
        }
        else if (!utility::mouse_down(EMouse::Left) && _is_controling_camera)
        {
            utility::window().release_mouse();
            _is_controling_camera = false;
            //_wantsInput = false;
            utility::window().warp_mouse(_locked_mouse_position);
        }
        else if(_is_controling_camera)
        {
            //utility::window().warp_mouse(_locked_mouse_position);

            real delta = utility::window().delta_time();
            //_wantsInput = false;

            vec2 mouse_motion = utility::window().mouse_motion();

            constexpr real mouse_sensitivity = 1.5f;

            rotation.y -= mouse_motion.x * mouse_sensitivity;
            rotation.x += mouse_motion.y * mouse_sensitivity;

            quat q(rotation);
            _algoCameraTransform.direction = q * NIGHT_CAMERA_DEFAULT_DIRECTION;

            constexpr real flight_speed = 4.0f;

            // TODO: translate according to direction
            if (utility::key_down(EKey::A))
            {
                //_algoCameraTransform.translation.x -= 1.0f * delta;
                _algoCameraTransform.translation += normalize(math::cross(_algoCameraTransform.direction, UP)) * flight_speed * delta;
            }

            if (utility::key_down(EKey::D))
            {
                //_algoCameraTransform.translation.x += 1.0f * delta;
                _algoCameraTransform.translation += normalize(math::cross(_algoCameraTransform.direction, DOWN)) * flight_speed * delta;
            }

            if (utility::key_down(EKey::Space))
            {
                //_algoCameraTransform.translation.x -= 1.0f * delta;
                _algoCameraTransform.translation += normalize(math::cross(_algoCameraTransform.direction, RIGHT)) * flight_speed * delta;
            }

            if (utility::key_down(EKey::LCtrl))
            {
                //_algoCameraTransform.translation.x += 1.0f * delta;
                _algoCameraTransform.translation += normalize(math::cross(_algoCameraTransform.direction, LEFT)) * flight_speed * delta;
            }

            if (utility::key_down(EKey::W))
            {
                //_algoCameraTransform.translation.y += 1.0f * delta;
                _algoCameraTransform.translation -= _algoCameraTransform.direction * flight_speed * delta;
            }

            if (utility::key_down(EKey::S))
            {
                //_algoCameraTransform.translation.y -= 1.0f * delta;
                _algoCameraTransform.translation += _algoCameraTransform.direction * flight_speed * delta;
            }

            _wantsInput = true;
        }

        gui.end_canvas();

        //gui.text("Camera:");
        //gui.drag_r32("X", &_algoCameraTransform.translation.x, 0.05f);
        //gui.drag_r32("Y", &_algoCameraTransform.translation.y, 0.05f);
        //gui.drag_r32("Z", &_algoCameraTransform.translation.z, 0.05f);
        //
        //
        //gui.drag_r32("Pitch", &rotation.x, 0.05f);
        //gui.drag_r32("Yaw", &rotation.y, 0.05f);
        //gui.drag_r32("Roll", &rotation.z, 0.05f);
        //quat q(rotation);
        //_algoCameraTransform.direction = q * NIGHT_CAMERA_DEFAULT_DIRECTION;
        //
        //
        //if (gui.button("Reset Camera"))
        //{
        //    _algoCameraTransform = {};
        //    rotation = vec3(0);
        //}
        
#if 0
        // TODO: use node tree to access algos, root node will display all algos in scene,
        // child node will display they're algos, etc.
        // no longer need to deal with involved nodes
        // TODO: add hide button for algo stack

        auto& gui = utility::gui();

        //ASSERT(_algoAlgorithmStack.empty()); // don't forget to call DB_ALGO_POP
        ASSERT(_algoCurrentlyInvolvedNodesStack.empty());// don't forget to call DB_ALGO_POP

        b8 is_viewing_step = false;

        for (auto& i : _algoRenderTargets)
        {
            ASSERT(i != nullptr);
            if (i->width() != utility::renderer().width() || i->height() != utility::renderer().height())
            {
                i->resize(ivec2(utility::renderer().width(), utility::renderer().height()));
            }

            i->clear(COLOR_ZERO);
        }

        // display node tree, allow nodes to be selected
        for (auto i = _algoInvolvedNodes.begin(); i != _algoInvolvedNodes.end(); i++)
        {
            if ((*i).first.empty())
            {
                if (gui.selectable("Nodeless", false))
                {

                }
            }

            string names;
            for (s32 j = 0; j < (*i).first.size(); j++)
            {
                if ((*i).first[j] == nullptr)
                {
                    continue;
                }

                names += (*i).first[j]->name() + "(" + to_string((*i).first[j]->unique_id()) + ")";
                if (j < (*i).first.size() - 1)
                {
                    names += ", ";
                }
            }

            if (names.empty())
            {
                continue;
            }

            gui.selectable(names, _guiSelectedInvolvedNodes == (*i).first);
            {
                if (gui.is_item_clicked())
                {
                    if (_guiSelectedInvolvedNodes != (*i).first)
                    {
                            
                        _guiSelectedInvolvedNodes = (*i).first;
                    }
                    else
                    {
                        _guiSelectedInvolvedNodes.clear();
                    }

                    _guiSelectedAlgorithm.clear();
                    _guiSelectedAlgorithmSteps.clear();
                }
            }
        }

        // drag step sliders
        if (!_guiSelectedInvolvedNodes.empty())// TODO: this wont work with Nodeless
        {
            _renderGraph.current_buffer(_algoRenderTargets[0], nullptr, nullptr);

            Color clear_color = BLACK; // TODO: member var
            clear_color.a = 0.95f;
            _renderGraph.draw_quad(QuadParams{ .position = {0.0f, 0.0f, 0.0f}, .size = {100, 100}/*its stupid but its debug*/, .color = clear_color});
                
            function<s32(map<string, Algorithm> const&, s32)> fn = [&](map<string, Algorithm> const& branches, s32 depth) -> s32
            {
                if (_guiSelectedAlgorithmSteps.size() <= depth)
                {
                    _guiSelectedAlgorithmSteps.emplace_back("", -1);
                }

                string& selected_name = _guiSelectedAlgorithmSteps[depth].first;
                s32& selected_step = _guiSelectedAlgorithmSteps[depth].second;

                string next_name = "";
                s32 inverse_depth = 0;

                for (auto i = branches.begin(); i != branches.end(); i++)
                {
                    string const& name = (*i).first;
                    Algorithm const& algo = (*i).second;

                    ASSERT(!name.empty());
 
                    if (name == selected_name)
                    {
                        s32 prev = selected_step;
                        string tabs;
                        for (s32 i = 0; i < depth; i++)
                        {
                            tabs += "  ";
                        }
                        gui.text(tabs);
                        gui.same_line();
                        gui.drag_s32(" " + name, &selected_step, 0.1f, -1, (s32)algo.render_steps.size() - 1);
                        selected_step = CLAMP(selected_step, -1, (s32)algo.render_steps.size() - 1);

                        // clear all steps ahead of selected
                        if (selected_step != prev && depth + 1 < _guiSelectedAlgorithmSteps.size())
                        {
                            for (s32 j = depth + 1; j < _guiSelectedAlgorithmSteps.size(); j++)
                            {
                                _guiSelectedAlgorithmSteps[j].first = "";
                                _guiSelectedAlgorithmSteps[j].second = -1;
                            }
                        }

                        if (selected_step == -1)
                        {
                            inverse_depth = -1;
                            continue;
                        }

                        auto& render_step = algo.render_steps[selected_step];
                        auto& callbacks = render_step.callbacks;
                        auto& sub_steps = render_step.sub_steps;

                        if (!sub_steps.empty())
                        {
                            inverse_depth = fn(sub_steps, depth + 1) + 1; // less depth, bigger number
                        }

                        if (!callbacks.empty())
                        {
                            ASSERT(inverse_depth + 1 < (s32)_algoRenderTargets.size());
                            ASSERT(_algoRenderTargets[inverse_depth + 1] != nullptr);
                            _renderGraph.current_buffer(_algoRenderTargets[inverse_depth + 1], nullptr, nullptr);

                            _isViewingAlgo = true;
                            for (const auto& i : callbacks)
                            {
                                i();
                            }
                            _isViewingAlgo = false;

                            is_viewing_step = true;
                        }
                    }
                    else
                    {
                        s32 dummy_step = -1;
                        string tabs;
                        for (s32 i = 0; i < depth; i++)
                        {
                            tabs += "  ";
                        }
                        gui.text(tabs);
                        gui.same_line();
                        gui.drag_s32("Step " + name, &dummy_step, 0.1f, -1, 0);

                        if (dummy_step == 0)
                        {
                            next_name = name;
                        }
                    }

                    if (!next_name.empty())
                    {
                        selected_name = next_name;
                        selected_step = 0;
                    }

                    if (selected_name.empty())
                    {
                        inverse_depth = -1;
                    }
                }

                return inverse_depth;
            };

            auto tree = _algoInvolvedNodes.find(_guiSelectedInvolvedNodes);

            if (tree != _algoInvolvedNodes.end())
            {
                gui.text("Algorithms: ");
                for (auto i = (*tree).second.begin(); i != (*tree).second.end(); i++)
                {
                    gui.selectable((*i).first, _guiSelectedAlgorithm == (*i).first);
                    if (gui.is_item_clicked())
                    {
                        if (_guiSelectedAlgorithm != (*i).first)
                        {
                            _guiSelectedAlgorithm = (*i).first;
                        }
                        else
                        {
                            _guiSelectedAlgorithm.clear();
                        }

                        _guiSelectedAlgorithmSteps.clear();
                    }
                }

                if (!_guiSelectedAlgorithm.empty())
                {
                    auto f = (*tree).second.find(_guiSelectedAlgorithm);
                    if (f != (*tree).second.end())
                    {
                        gui.text("Steps: ");

                        map<string, Algorithm> dummy_map; // TODO: this is dumb.
                        dummy_map.insert({ (*f).first, (*f).second });
                        fn(dummy_map, 0);
                    }

                    //fn((*tree).second, 0);
                }
            }

            {
                // render darkness
                _renderGraph.current_buffer(utility::renderer().default_render_target(), nullptr, vector<handle<const ITexture>>(1, _algoRenderTargets[0]));

                s32 w = _algoRenderTargets[0]->width(); // TODO: maybe use NodeFrameBuffer
                s32 h = _algoRenderTargets[0]->height();
                vec2 ar = { (h < w ? (real)h / (real)w : 1.0f), (w < h ? (real)w / (real)h : 1.0f) };
                vec3 scale = vec3(1.0f / ar.x, 1.0f / ar.y, 1.0f);
                _renderGraph.draw_quad(QuadParams{ .position = {0, 0, 0.5f}, .size = scale }); // lower alpha
            }
        }

        if (!is_viewing_step)
        {
            return;
        }

        // TODO: render to max depth.
        for (s32 i = 1; i < _algoRenderTargets.size(); i++)
        {
            _renderGraph.current_buffer(utility::renderer().default_render_target(), nullptr, vector<handle<const ITexture>>(1, _algoRenderTargets[i]));

            real depth = 0.001f * i;
            Color color = WHITE;
            color.a /= (i - 1) * 4 + 1;

            s32 w = _algoRenderTargets[i]->width(); // TODO: maybe use NodeFrameBuffer
            s32 h = _algoRenderTargets[i]->height();
            vec2 ar = { (h < w ? (real)h / (real)w : 1.0f), (w < h ? (real)w / (real)h : 1.0f) };
            vec3 scale = vec3(1.0f / ar.x, 1.0f / ar.y, 1.0f);
            _renderGraph.draw_quad(QuadParams{ .position = {0, 0, 0.5f + depth}, .size = scale, .color = color }); // lower alpha
        }

        _guiBreak = {};

        if (gui.button("Break")) // check if step is selected
        {
            for (s32 i = 0; i < _guiSelectedAlgorithmSteps.size(); i++)
            {
                auto& step = _guiSelectedAlgorithmSteps[i];
                if (step.second == -1)
                {
                    break;
                }

                //_guiBreak.step = step.second; // TODO: do something better
                //_guiBreak.algo_stack.push_back(step.first);
                _guiBreak.algorithm_stack.push_back(step);
            }


            _guiBreak.involved_nodes = _guiSelectedInvolvedNodes;
        }
#endif
    }

    void DebugRenderer::profiler_render()
    {
        auto& gui = utility::gui();
        auto& root = Profiler::root();

        if (gui.button("Clear"))
        {
            // clear all profiler timers below the root layer.
            for (auto& i : Profiler::_root.children)
            {
                i.second.children.clear();
            }
        }

        function<void(string const&, Profiler::Node const&, s32)> rec = [&](string const& name, Profiler::Node const& node, s32 tabs)
        {
            if (node.timer_history.empty())
            {
                return;
            }

            string t = "";

            for (s32 i = 0; i < tabs; i++)
            {
                t += "   ";
            }

            // TODO: handle this like the smoothing in canvas
            u32 dur_count = 0;
            //u64 dur_sum = 0;
            //for (s32 i = 0; i < node.timer_history.size(); i++)
            //{
            //    dur_sum += node.timer_history[i].duration;
            //    dur_count++;
            //}
            //
            //if (dur_count > 0)
            //{
            //    dur_sum /= dur_count;
            //}
            //
            //dur_sum = node.timer_history.back().duration;
            
            r64 time_elapsed = utility::window().time_elapsed();
            r64 delta_time = time_elapsed - node.timer_history.back().timestamp;
            Color color = Color::lerp(node.color, node.color.darken(0.85f), CLAMP((r32)delta_time, 0, 1));

            gui.text_colored(t + name + ": " + to_string((s32)node.average_time), color);

            for (const auto& i : node.children)
            {
                rec(i.first, i.second, tabs + 1);
            }
        };

        for (const auto& i : root.children)
        {
            rec(i.first, i.second, 0);
        }

        //rec("Root", root, 0);

        //for (const auto& i : timers)
        //{
        //    gui.text(i.first + ": " + to_string(i.second.duration));
        //}
    }

    void DebugRenderer::assets_render()
    {
        //_renderGraph.current_buffer(_renderTarget, nullptr, nullptr);

        auto& gui = utility::gui();

        gui.checkbox("Should Save Debug Textures", &_assetsShouldSaveDebugTextures);

        {
            // render darkness
            _renderGraph.current_buffer(utility::renderer().default_render_target(), nullptr, nullptr);
            _renderGraph.append_transform(mat4(1));

            s32 w = utility::renderer().default_render_target()->width(); // TODO: maybe use NodeFrameBuffer
            s32 h = utility::renderer().default_render_target()->height();
            vec2 ar = { (h < w ? (real)h / (real)w : 1.0f), (w < h ? (real)w / (real)h : 1.0f) };
            vec3 scale = vec3(1.0f / ar.x, 1.0f / ar.y, 1.0f);

            Quad q = Quad(QuadParams<>{ .position = ORIGIN, .size = scale, .color = PURPLE.opaqued(0.95f) });
            _renderGraph.draw_quad(q); // lower alpha
            utility::renderer().flush_render_graph(_renderGraph);
            utility::renderer().flush();
            _renderGraph.clear();
            _renderGraph.append_transform(mat4(1));
        }

        auto& textures = utility::renderer()._textures;

        if (gui.selectable("Show Depth Buffer", _assetsTextureShowDepthBuffer))
        {
            _assetsTextureShowDepthBuffer = !_assetsTextureShowDepthBuffer;
        }

        gui.seperator();

        for (const auto& i : textures)
        {
            if (gui.selectable(i.first, i.first == _assetsSelectedTexture))
            {
                if (i.first == _assetsSelectedTexture)
                {
                    _assetsSelectedTexture.clear();
                }
                else
                {
                    _assetsSelectedTexture = i.first;
                }
            }
        }

        if (!_assetsSelectedTexture.empty())
        {
            auto f = textures.find(_assetsSelectedTexture);
            if (f != textures.end())
            {
                Quad q = QuadParams{ .position = FORWARD * (real)20, .size = vec2(1) };

                if (!_assetsTextureShowDepthBuffer)
                {
                    _renderGraph.current_buffer(
                        utility::renderer().default_render_target(),
                        nullptr,
                        (*f).second
                    );
                }
                else
                {
                    _renderGraph.current_buffer(
                        utility::renderer().default_render_target(),
                        nullptr,
                        (*f).second->depth_buffer()
                    );
                }

                _renderGraph.draw_quad(q);
                utility::renderer().flush_render_graph(_renderGraph);
                utility::renderer().flush();
                _renderGraph.clear();
                _renderGraph.append_transform(mat4(1));
            }
            else
            {
                _assetsSelectedTexture.clear();
            }
        }
    }

    void DebugRenderer::draw_sphere(DrawSphereParams const& params, b8 is_algo)
    {
        push_draw_function([=]()
            {
                DrawSphereParams2 dsp;
                dsp.transform = math::compose({.translation = params.origin});
                dsp.radius = params.radius;
                dsp.color = params.color;
                dsp.on_draw_line = [&](DrawLineCallbackParams const& dlcp)
                    {
                        _renderGraph.draw_line(dlcp.params);
                    };

                //::night::DrawSphereParams dsp;
                //dsp.origin = params.origin;
                //dsp.radius = params.radius;
                //dsp.color = params.color;
                ////dsp.wireframe = false;
                ////dsp.outline_only = true;
                //dsp.width = RENDERER_LINE_DEFAULT_WIDTH;
                //dsp.out_graph = &_renderGraph;
                //
                //ShapeRenderer3D::draw_sphere(dsp);
            }, is_algo);
    }

    void DebugRenderer::draw_cylinder(DrawCylinderParams const& params, b8 is_algo)
    {
        // TODO: fix edge case where cylinder lies on camera direction.
        //vec3 direction = this->direction();
        //vec3 origin = translation();
        //real radius = radius_scaled();
        push_draw_function([=]()
            {
                ASSERT(_renderTarget != nullptr);
                Camera const& camera = _renderTarget->camera();
                vec3 camera_direction = camera.direction();

                vec3 cap_near = params.origin + params.direction * params.height;
                vec3 cap_far = params.origin - params.direction * params.height;

                if (math::dot(cap_far - cap_near, camera_direction) > 0.0f)
                {
                    SWAP(cap_near, cap_far);
                }

                mat4 forward_to_direction = math::rotate_about_vector(FORWARD, params.direction);

                // draw the body:
                vec3 dxcd = math::normalize(math::cross(params.direction, camera_direction));
                _renderGraph.draw_line(cap_near + dxcd * params.radius, cap_far + dxcd * params.radius, params.color);
                _renderGraph.draw_line(cap_near - dxcd * params.radius, cap_far - dxcd * params.radius, params.color);

                // draw caps
                for (s32 i = 0; i < params.segments; i++)
                {
                    real t1 = (real)i / (real)(params.segments - 1) * R_PI * 2;
                    real t2 = (real)(i + 1) / (real)(params.segments - 1) * R_PI * 2;

                    vec3 p1;
                    p1.x = cos(t1) * params.radius;
                    p1.y = sin(t1) * params.radius;
                    p1.z = 0;

                    vec3 p2;
                    p2.x = cos(t2) * params.radius;
                    p2.y = sin(t2) * params.radius;
                    p2.z = 0;

                    p1 = forward_to_direction * vec4(p1, 1);
                    p2 = forward_to_direction * vec4(p2, 1);

                    vec3 np1 = p1 + cap_near;
                    vec3 np2 = p2 + cap_near;

                    _renderGraph.draw_line(np1, np2, params.color);

                    vec3 fp1 = p1 + cap_far;
                    vec3 fp2 = p2 + cap_far;

                    if (math::dot(cap_far - fp1, camera_direction) < 0.0f)
                    {
                        _renderGraph.draw_line(fp1, fp2, params.color);
                    }
                    else
                    {
                        _renderGraph.draw_line(fp1, fp2, params.color.opaqued(0.15f)); // TODO: use theta
                    }
                }
            }, is_algo);
    }

    IGui& DebugRenderer::gui()
    {
        return utility::gui();
    }

#if 0
    void DebugRenderer::camera(Camera const& camera)
    {
        //for (auto& i : _algoRenderTargets)
        // TODO: make darkness texture a seperate render target.
        for(s32 i = 1; i < _algoRenderTargets.size(); i++)
        {
            ASSERT(_algoRenderTargets[i] != nullptr);
            _algoRenderTargets[i]->camera(camera);
        }

        ASSERT(_sceneRenderTarget != nullptr);
        _sceneRenderTarget->camera(camera);
    }
#endif

    b8 DebugRenderer::algo_should_break()
    {
        if (_guiBreak.algorithm_stack.empty())
        {
            return false;
        }

        ASSERT(!_algoCurrentlyInvolvedNodesStack.empty());
        auto& top = _algoCurrentlyInvolvedNodesStack.back();
        ASSERT(!top.involved_nodes.empty());
        ASSERT(!top.algorithm_stack.empty());

        if (top.involved_nodes != _guiBreak.involved_nodes)
        {
            return false;
        }

        if (top.algorithm_stack.size() != _guiBreak.algorithm_stack.size())
        {
            return false;
        }

        for (s32 i = 0; i < top.algorithm_stack.size(); i++)
        {
            if (_guiBreak.algorithm_stack[i].first != top.algorithm_stack[i])
            {
                return false;
            }
        }

        auto& nodes = _algoInvolvedNodes[top.involved_nodes];
        auto* curr = &nodes[_guiBreak.algorithm_stack[0].first];

        if (curr->render_steps.size() != _guiBreak.algorithm_stack[0].second + 1)
        {
            return false;
        }

        for (s32 i = 1; i < _guiBreak.algorithm_stack.size(); i++)
        {
            if (curr->render_steps.empty())
            {
                break;
            }

            auto& algo = _guiBreak.algorithm_stack[i].first;
            curr = &curr->render_steps.back().sub_steps[algo];

            if (curr->render_steps.size() != _guiBreak.algorithm_stack[i].second + 1)
            {
                return false;
            }
        }

        return true;
    }

    void DebugRenderer::push_draw_function(function<void()> fn, b8 is_algo)
    {
        if (_isViewingAlgo)
        {
            ASSERT(fn != nullptr);
            fn();
            return;
        }

        if (is_algo)
        {
            if (!_algoIsActive || _algoCurrentlyInvolvedNodesStack.empty())
            {
                return;
            }

            // TODO: exit if step, algorithm and involved nodes are not selected, exit in macro so we don't copy vectors every iteration.
            // can only exit after 1 algo push or we can't see algos that are called once on scene construction.
            //auto& nodes = _algoInvolvedNodes[_algoCurrentlyInvolvedNodes];

            auto& top = _algoCurrentlyInvolvedNodesStack.back();
            ASSERT(!top.involved_nodes.empty());
            ASSERT(!top.algorithm_stack.empty());

            auto& nodes = _algoInvolvedNodes[top.involved_nodes];
            auto* curr = &(nodes[top.algorithm_stack[0]]);
            //auto* curr = &nodes[_algoAlgorithmStack[0]];

            for (s32 i = 1; i < top.algorithm_stack.size(); i++)
            {
                auto& algo = top.algorithm_stack[i];

                ASSERT(!curr->render_steps.empty());

                curr = &curr->render_steps.back().sub_steps[algo];
            }

            if (curr->render_steps.empty())
            {
                curr->render_steps.push_back({});
            }

            curr->render_steps.back().callbacks.push_back(fn);
        }
        else if(!is_algo)
        {
            ASSERT(fn != nullptr);
            fn();
        }
    }

    void DebugRenderer::draw_point(DrawPointParams const& params, b8 is_algo)
    {
        push_draw_function([=]()
        {
            render_graph().draw_point(params.point, params.color);
        }, is_algo);
    }

    void DebugRenderer::draw_line(DrawLineParams const& params, b8 is_algo)
    {
        push_draw_function([=]()
        {
            render_graph().draw_line(params);
        }, is_algo);
    }

    void DebugRenderer::draw_quad(Quad<> const& quad, b8 is_algo)
    {
        push_draw_function([=]()
            {
                render_graph().draw_quad(quad);
            }, is_algo);
    }

    void DebugRenderer::draw_text(Text const& text, b8 is_algo)
    {
        // TODO: re-impl
#if 0
        push_draw_function([=]()
            {
                render_graph().draw_text(text);
            }, is_algo);
#endif
    }

    void DebugRenderer::draw_hover_text(DrawHoverTextParams const& params, b8 is_algo)
    {
        ASSERT(false); // TODO: impl
#if 0
        push_draw_function([=]()
            {
                Ray ray;
                vec3 pick;
                vec2 res;
                vec3 point_pick;
                auto& render_target = _renderGraph.current_render_target();

                if (render_target == nullptr) // TODO: need to render something if no render target
                {
                    ray = utility::renderer().default_render_target()->mouse_pick(utility::window().mouse());
                    pick = utility::renderer().default_render_target()->project_to_screen(ray.origin);
                    point_pick = utility::renderer().default_render_target()->project_to_screen(params.origin);
                    res = { utility::renderer().width(), utility::renderer().height() };
                }
                else
                {
                    ray = render_target->mouse_pick(utility::window().mouse());
                    pick = render_target->project_to_screen(ray.origin);
                    point_pick = render_target->project_to_screen(params.origin);
                    res = { render_target->width(), render_target->height() };
                }

                //TRACE(pick.x, ", ", pick.y);

                // TODO: handle this
                vec2 ar = { (res.y < res.x ? (real)res.y / (real)res.x : 1.0f), (res.x < res.y ? (real)res.x / (real)res.y : 1.0f) };

                if (distance(vec2(point_pick) / ar, vec2(pick) / ar) > params.hover_radius)
                {
                    return;
                }

                auto font = utility::renderer().find_texture("Default Font");
                if (font != nullptr)
                {
                    Text text;
                    text.text = params.text;
                    text.color = params.color;
                    text.font = font;
                    text.origin = ETextOrigin::BottomCenter;
                    text.transform = glm::translate(vec3(point_pick.x, point_pick.y + 0.05f, 0.999f));
                    text.transform *= glm::scale(vec3(0.015f, 0.015f, 1.0f));

                    _renderGraph.draw_text(text);
                }
            }, is_algo);
#endif
    }

    void DebugRenderer::draw_arrow(DrawArrowParams const& params, b8 is_algo)
    {
        push_draw_function([=]()
            {
                vec3 direction = math::normalize(params.direction);
                _renderGraph.draw_line(params.origin, params.origin + direction * params.length, params.color);
                _renderGraph.draw_point(params.origin + direction * params.length * (real)0.95, WHITE); // TODO: draw 3D arrow
            }, is_algo);
    }

    void DebugRenderer::draw_plane(DrawPlaneParams const& params, b8 is_algo)
    {
        push_draw_function([=]()
            {
                mat4 forward_to_normal = math::rotate_about_vector(FORWARD, params.normal);

                Color grid_color = params.color;
                //grid_color.a *= 0.5f;

                for (s32 y = 0; y < params.grid_resolution - 1; y++)
                {
                    real ty1 = (real)y / (params.grid_resolution - 1);
                    real ty2 = (real)(y + 1) / (params.grid_resolution - 1);

                    for (s32 x = 0; x < params.grid_resolution - 1; x++)
                    {
                        real tx1 = (real)x / (params.grid_resolution - 1);
                        real tx2 = (real)(x + 1) / (params.grid_resolution - 1);

                        vec3 p1 = forward_to_normal * vec4(math::lerp(-params.grid_size, params.grid_size, tx1), math::lerp(-params.grid_size, params.grid_size, ty1), 0.0f, 1.0f);
                        vec3 p2 = forward_to_normal * vec4(math::lerp(-params.grid_size, params.grid_size, tx2), math::lerp(-params.grid_size, params.grid_size, ty1), 0.0f, 1.0f);
                        vec3 p3 = forward_to_normal * vec4(math::lerp(-params.grid_size, params.grid_size, tx2), math::lerp(-params.grid_size, params.grid_size, ty2), 0.0f, 1.0f);
                        vec3 p4 = forward_to_normal * vec4(math::lerp(-params.grid_size, params.grid_size, tx1), math::lerp(-params.grid_size, params.grid_size, ty2), 0.0f, 1.0f);

                        p1 += params.origin;
                        p2 += params.origin;
                        p3 += params.origin;
                        p4 += params.origin;

                        _renderGraph.draw_line(p1, p2, grid_color, RENDERER_LINE_DEFAULT_WIDTH * 0.666f);
                        _renderGraph.draw_line(p2, p3, grid_color, RENDERER_LINE_DEFAULT_WIDTH * 0.666f);
                        _renderGraph.draw_line(p3, p4, grid_color, RENDERER_LINE_DEFAULT_WIDTH * 0.666f);
                        _renderGraph.draw_line(p4, p1, grid_color, RENDERER_LINE_DEFAULT_WIDTH * 0.666f);
                    }
                }

                draw_arrow({ .origin = params.origin, .direction = params.normal, .color = params.color });
            }, is_algo);
    }

    void DebugRenderer::draw_ellipse(DrawEllipseParams const& params, b8 is_algo)
    {
        push_draw_function([=]()
            {
                vec3 major_axis_n = math::normalize(params.major_axis);
                real major_axis_len = math::length(params.major_axis);
                vec3 normal = math::normalize(params.normal);

                mat4 up_to_normal = math::rotate_about_vector(UP, normal);

                mat4 normal_to_major_axis = math::rotate_about_vector(normal, major_axis_n);

                vec3 up_on_ellipse = normal_to_major_axis * up_to_normal * vec4(UP, 1);
                vec3 right_on_ellipse = normal_to_major_axis * up_to_normal * vec4(RIGHT, 1);
                vec3 uxr = math::normalize(math::cross(up_on_ellipse, right_on_ellipse));

                mat4 uxr_to_normal = math::rotate_about_vector(uxr, normal);

                for (s32 i = 0; i < params.segments - 1; i++)
                {
                    real t1 = (real)i / (real)(params.segments - 1);
                    real t2 = (real)(i + 1) / (real)(params.segments - 1);
                    t1 = math::lerp(params.min_theta, params.max_theta, t1);
                    t2 = math::lerp(params.min_theta, params.max_theta, t2);

                    vec3 p1;
                    p1.x = sin(t1) * params.radii.x;
                    p1.y = cos(t1) * params.radii.y * major_axis_len;
                    p1.z = 0;

                    vec3 p2;
                    p2.x = sin(t2) * params.radii.x;
                    p2.y = cos(t2) * params.radii.y * major_axis_len;
                    p2.z = 0;

                    p1 = up_to_normal * vec4(p1, 1);
                    p2 = up_to_normal * vec4(p2, 1);
                    p1 = normal_to_major_axis * vec4(p1, 1);
                    p2 = normal_to_major_axis * vec4(p2, 1);
                    p1 = uxr_to_normal * vec4(p1, 1);
                    p2 = uxr_to_normal * vec4(p2, 1);

                    p1 += params.origin;
                    p2 += params.origin;

                    _renderGraph.draw_line(p1, p2, params.color);
                }
            }, is_algo);
    }
};

#endif