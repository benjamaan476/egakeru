#include "render_graph.h"
#include <algorithm>
#include "engine/engine.h"

namespace egkr
{
    rendergraph::pass::pass() { event::register_event(event::code::render_target_refresh_required, this, on_event); }
    bool rendergraph::pass::init() { return false; }
    bool rendergraph::pass::execute(const frame_data& /*frame_data*/) const { return false; }

    bool rendergraph::pass::destroy() 
    {
	return true; 
    }

    bool rendergraph::pass::regenerate_render_targets()
    {
	for (uint8_t i{0u}; i < renderpass->get_render_target_count(); i++)
	{
	    auto& target = renderpass->get_render_target(i);
	    //target->destroy();

	    for (uint32_t a{0u}; a < target->get_attachments().size(); a++)
	    {
		auto& attachment = target->get_attachments()[a];
		if (attachment.source == render_target::attachment_source::default_source)
		{
		    if (attachment.type == render_target::attachment_type::colour)
		    {
			attachment.texture_attachment = engine::get()->get_renderer()->get_backend()->get_window_attachment(i);
		    }
		    else if (attachment.type == render_target::attachment_type::depth)
		    {
			attachment.texture_attachment = engine::get()->get_renderer()->get_backend()->get_depth_attachment(i);
		    }
		    else
		    {
			LOG_ERROR("Attachment type not supported");
			return false;
		    }
		}
		else if (attachment.source == render_target::attachment_source::view)
		{
		    LOG_ERROR("Cant do this");
		    return false;
		}
	    }
	    const auto& attachments = target->get_attachments();
	    target = render_target::render_target::create(attachments, renderpass.get(), attachments[0].texture_attachment->get_width(), attachments[0].texture_attachment->get_height());
	}
	return true;
    }

    bool rendergraph::pass::on_event(event::code code, void*, void* listener, const event::context& /*context*/)
    {
	auto* view = static_cast<pass*>(listener);
	switch (code)
	{
	case event::code::render_target_refresh_required:
	    view->regenerate_render_targets();
	    break;
	default:
	    break;
	}
	return false;
    }

    rendergraph::rendergraph(const std::string& rendergraph_name): name{rendergraph_name} { }

    rendergraph rendergraph::create(const std::string& rendergraph_name) { return rendergraph(rendergraph_name); }

    bool rendergraph::destroy()
    {
	for (const auto& renderpass : passes)
	{
	    renderpass->destroy();
	    delete renderpass;
	}

	for (auto& global_source : global_sources)
	{
	    for (auto& texture : global_source.textures)
	    {
		texture->destroy();
		texture.reset();
	    }
	}
	passes.clear();
	return false;
    }

    bool rendergraph::add_gloabl_source(const std::string& global_source_name, source::type type, source::origin origin)
    {
	global_sources.emplace_back(source{.name = global_source_name, .source_type = type, .source_origin = origin});

	return false;
    }

    rendergraph::pass* rendergraph::create_pass(const std::string& pass_name, std::function<pass*()> init)
    {
	for (const auto& renderpass : passes)
	{
	    if (renderpass->get_name() == pass_name)
	    {
		LOG_ERROR("Pass with name {} already exists", pass_name);
		return nullptr;
	    }
	}

	auto* renderpass = init();
	if (renderpass == nullptr)
	{
	    LOG_ERROR("Failed to initialise pass {}", pass_name);
	    return nullptr;
	}

	passes.push_back(renderpass);
	return renderpass;
    }

    bool rendergraph::add_source(const std::string& pass_name, const std::string& source_name, source::type type, source::origin origin)
    {
	for (auto& renderpass : passes)
	{
	    if (renderpass->get_name() == pass_name)
	    {
		for (const auto& src : renderpass->get_sources())
		{
		    if (src.name == source_name)
		    {
			LOG_ERROR("Source with name {} already exists in pass {}", src.name, renderpass->get_name());
			return false;
		    }
		}
		renderpass->get_sources().emplace_back(rendergraph::source{.name = source_name, .source_type = type, .source_origin = origin});
	    }
	}
	return true;
    }

    bool rendergraph::add_sink(const std::string& pass_name, const std::string& sink_name)
    {
	for (auto& renderpass : passes)
	{
	    if (renderpass->get_name() == pass_name)
	    {
		for (const auto& s : renderpass->get_sinks())
		{
		    if (s.name == sink_name)
		    {
			LOG_ERROR("Sink with name {} already exists in pass {}", s.name, renderpass->get_name());
			return false;
		    }
		}
		renderpass->get_sinks().emplace_back(sink{.name = sink_name});
	    }
	}

	return true;
    }

    bool rendergraph::set_sink_linkage(const std::string& pass_name, const std::string& sink_name, const std::string& source_pass_name, const std::string& source_name)
    {
	auto renderpass = std::ranges::find_if(passes, [pass_name](const auto& p) { return p->get_name() == pass_name; });
	auto snk = std::ranges::find_if((*renderpass)->get_sinks(), [sink_name](const auto& s) { return s.name == sink_name; });

	if (source_pass_name == "")
	{
	    auto sourcee = std::ranges::find_if(global_sources, [source_name](const auto& s) { return s.name == source_name; });
	    snk->bound_source = &*sourcee;
	}
	else
	{
	    auto source_pass = std::ranges::find_if(passes, [source_pass_name](const auto& p) { return p->get_name() == source_pass_name; });
	    auto src = std::ranges::find_if((*source_pass)->get_sources(), [source_name](const auto& s) { return s.name == source_name; });
	    snk->bound_source = &*src;
	}
	return true;
    }

    bool rendergraph::finalise()
    {
	for (auto& global_source : global_sources)
	{
	    if (global_source.source_origin == source::origin::global)
	    {
		for (uint8_t i{0u}; i < engine::get()->get_renderer()->get_backend()->get_window_attachment_count(); i++)
		{
		    if (global_source.source_type == source::type::render_target_colour)
		    {
			global_source.textures.push_back(engine::get()->get_renderer()->get_backend()->get_window_attachment(i));
		    }
		    else if (global_source.source_type == source::type::render_target_depth_stencil)
		    {
			global_source.textures.push_back(engine::get()->get_renderer()->get_backend()->get_depth_attachment(i));
		    }
		    else
		    {
			LOG_ERROR("Source type not supported");
			return false;
		    }
		}
	    }
	}

	const pass* backbuffer_first_use{};

	for (const auto& renderpass : passes)
	{
	    for (const auto& snk : renderpass->get_sinks())
	    {
		auto& src = snk.bound_source;
		if (src->source_origin == source::origin::global && src->source_type == source::type::render_target_colour)
		{
		    backbuffer_first_use = renderpass;
		    break;
		}
	    }
	}

	if (!backbuffer_first_use)
	{
	    LOG_ERROR("No pass uses the backbuffer as a source");
	    return false;
	}

	for (auto& renderpass : passes)
	{
	    for (auto& src : renderpass->get_sources())
	    {
		if (src.source_type == source::type::render_target_colour && src.source_origin == source::origin::other)
		{
		    for (const auto& other_pass : passes)
		    {
			bool found = false;
			for (const auto& other_sink : other_pass->get_sinks())
			{
			    if (other_sink.bound_source == &src)
			    {
				found = true;
				break;
			    }
			}

			if (!found)
			{
			    backbuffer_global_sink.bound_source = &src;
			    renderpass->set_present_after(true);
			    break;
			}
		    }
		}
		if (backbuffer_global_sink.bound_source != nullptr)
		{
		    break;
		}
	    }
	    if (backbuffer_global_sink.bound_source != nullptr)
	    {
		break;
	    }
	}

	if (backbuffer_global_sink.bound_source == nullptr)
	{
	    LOG_ERROR("Unable to link backbuffer_global_sink to a source");
	    return false;
	}

	for (auto renderpass : passes)
	{
	    if (!renderpass->init())
	    {
		LOG_ERROR("Failed to initialise pass {}", renderpass->get_name());
		return false;
	    }

	    if (!renderpass->regenerate_render_targets())
	    {
		LOG_ERROR("Failed to regenerate render targets for pass {}", renderpass->get_name());
		return false;
	    }
	}
	return true;
    }

    bool egkr::rendergraph::execute(const frame_data& frame_data)
    {
	for (const auto& renderpass : passes)
	{
	    if (!renderpass->execute(frame_data))
	    {
		LOG_ERROR("Failed to execute pass {}", renderpass->get_name());
		return false;
	    }
	}
	return false;
    }
}
