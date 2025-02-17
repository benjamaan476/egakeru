#pragma once

#include "pch.h"
#include "resources/texture.h"
#include "renderpass.h"
#include "viewport.h"
#include <application/application.h>
#include <event.h>

namespace egkr
{
    class rendergraph
    {
    public:
	struct source
	{
	    enum type : uint8_t
	    {
		render_target_colour,
		render_target_depth_stencil,
	    };

	    enum class origin : uint8_t
	    {
		global,
		other,
		self
	    };

	    std::string name;
	    std::vector<texture::shared_ptr> textures;
	    type source_type;
	    origin source_origin;
	};

	struct sink
	{
	    std::string name;
	    source* bound_source;
	};

	class pass
	{
	public:
	    virtual ~pass() = default;
	    pass();
	    virtual bool init() = 0;
	    [[nodiscard]] virtual bool execute(const frame_data& frame_data) const = 0;
	    virtual bool destroy() = 0;

	    bool regenerate_render_targets();

	    void set_present_after(bool present) { present_after = present; }

	    [[nodiscard]] const std::string& get_name() const { return name; }
	    [[nodiscard]] const auto& get_sources() const { return sources; }
	    auto& get_sources() { return sources; }
	    [[nodiscard]] const auto& get_sinks() const { return sinks; }
	    auto& get_sinks() { return sinks; }

	    viewport* viewport;
	    float4x4 view;
	    float4x4 projection;
	    float3 view_position;
	    bool do_execute;
	protected:
	    std::string name;
	    std::vector<source> sources;
	    std::vector<sink> sinks;
	    renderpass::renderpass::shared_ptr renderpass;
	    bool present_after{false};
	private:
	    static bool on_event(event::code code, void* /*sender*/, void* listener, const event::context& context);
	};

	rendergraph() = default;
	explicit rendergraph(const std::string& rendergraph_name);

	static rendergraph create(const std::string& rendergraph_name);
	bool destroy();

	bool add_gloabl_source(const std::string& rendergrglobal_source_nameaph_name, source::type type, source::origin origin);
	pass* create_pass(const std::string& name, std::function<pass*()> init);

	bool add_source(const std::string& pass_name, const std::string& source_name, source::type type, source::origin origin);
	bool add_sink(const std::string& pass_name, const std::string& sink_name);
	bool set_sink_linkage(const std::string& pass_name, const std::string& sink_name, const std::string& source_pass_name, const std::string& source_name);

	bool finalise();
	bool execute(const frame_data& frame_data);
    private:
	std::string name;
	std::vector<source> global_sources;
	std::vector<pass*> passes;
	sink backbuffer_global_sink{};
    };
}
