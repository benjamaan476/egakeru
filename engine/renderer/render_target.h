#pragma once
#include "pch.h"

#include "resources/texture.h"

namespace egkr
{
    namespace renderpass
    {
	class renderpass;
    }
    namespace render_target
    {
	enum class attachment_type : uint8_t
	{
	    colour = 0x01,
	    depth = 0x02,
	    stencil = 0x04,
	};

	enum class attachment_source : uint8_t
	{
	    default_source,
	    self
	};

	enum class load_operation : uint8_t
	{
	    dont_care,
	    load
	};

	enum class store_operation : uint8_t
	{
	    dont_care,
	    store
	};

	struct attachment_configuration
	{
	    attachment_type type;
	    attachment_source source;
	    load_operation load_op;
	    store_operation store_op;
	    bool present_after;
	};

	struct attachment
	{
	    attachment_type type;
	    attachment_source source;
	    load_operation load_op;
	    store_operation store_op;
	    bool present_after;
	    texture::shared_ptr texture_attachment;
	};

	struct configuration
	{
	    egkr::vector<attachment_configuration> attachments;
	};

	class render_target
	{
	public:
	    using shared_ptr = std::shared_ptr<render_target>;

	    static shared_ptr create(const egkr::vector<attachment>& attachments, renderpass::renderpass* pass, uint32_t width, uint32_t height);
	    static shared_ptr create(const egkr::vector<attachment_configuration>& attachments);
	    render_target();

	    [[nodiscard]] const auto& get_attachments() const { return attachments_; }
	    auto& get_attachments() { return attachments_; }
	    virtual ~render_target();
	    virtual bool free(bool free_internal_memory) = 0;

	    void destroy();
	protected:
	    egkr::vector<attachment> attachments_;
	};
    }
}
