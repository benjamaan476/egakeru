#include "shader.h"
#include "shader.h"
#include "shader.h"
#include <format>
#include <ranges>

#include "platform/filesystem.h"
#include "vulkan_types.h"

namespace egkr
{

	static std::unordered_map<shader_stages, std::string> stage_by_shader_stages = {
		{shader_stages::frag, "frag"},
		{shader_stages::vert, "vert"}
	};

	std::string shader_stage_to_string(shader_stages stage)
	{
		return stage_by_shader_stages[stage];
	}

	shader_stage shader::create_shader_module(const vulkan_context* context, std::string_view shader_name, shader_stages stage, std::string_view shader_type)
	{
		std::string shader_stage{};
		vk::ShaderStageFlagBits flag{};

		switch (stage)
		{
		case shader_stages::frag:
			shader_stage = "frag";
			flag = vk::ShaderStageFlagBits::eFragment;
			break;
		case shader_stages::vert:
			shader_stage = "vert";
			flag = vk::ShaderStageFlagBits::eVertex;
			break;
		default:
			LOG_ERROR("Unknown shader stage used");
			return {};
		}

		const auto* source_dir = "../assets/shaders/";
		auto shader_filename = std::format("{}{}.{}.{}", source_dir, shader_name, shader_stage, shader_type);

		auto file = egkr::filesystem::open(shader_filename, egkr::file_mode::read, true);

		if (!file.is_valid)
		{
			LOG_ERROR("Failed to open file: {}", shader_filename);
			return {};
		}

		const auto code = filesystem::read_all(file);


		vk::ShaderModuleCreateInfo create_info{};
		create_info
			.setCodeSize(code.size())
			.setPCode((const uint32_t*)(code.data()));

		vk::ShaderModule shader_module = context->device.logical_device.createShaderModule(create_info, context->allocator);



		vk::PipelineShaderStageCreateInfo stage_create_info{};
		stage_create_info
			.setModule(shader_module)
			.setPName("main")
			.setStage(flag);

		LOG_INFO("Succesfully created {} shader", shader_stage);

		return {create_info, shader_module, stage_create_info};
	}

	shader::shared_ptr shader::create(const vulkan_context* context)
	{
		return std::make_shared<shader>(context);
	}

	shader::shader(const vulkan_context* context)
		: context_{context}
	{
		auto frag_shader_module = create_shader_module(context, "Builtin.ObjectShader"sv, shader_stages::frag, "spv"sv);
		auto vert_shader_module = create_shader_module(context, "Builtin.ObjectShader"sv, shader_stages::vert, "spv"sv);

		stages_[shader_stages::frag] = frag_shader_module;
		stages_[shader_stages::vert] = vert_shader_module;

	}

	shader::~shader()
	{
		destroy();
	}

	void shader::use()
	{
		const auto& image_index = context_->image_index;
		context_->graphics_command_buffers[image_index].get_handle().bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline_->get_handle());
	}

	void shader::set_pipeline(pipeline::shared_ptr pipeline)
	{
		pipeline_ = pipeline;
	}

	const egkr::vector<vk::PipelineShaderStageCreateInfo> shader::get_shader_stages() const
	{

		egkr::vector<vk::PipelineShaderStageCreateInfo> stages{};

		for (const auto& stage : stages_ | std::views::values)
		{
			stages.emplace_back(stage.stage_create_info);
		}

		return stages;
	}

	void shader::destroy()
	{
		if (pipeline_)
		{
			pipeline_->destroy();
		}

		for (auto& stage : stages_)
		{
			context_->device.logical_device.destroyShaderModule(stage.second.handle, context_->allocator);
		}
		stages_.clear();
	}
}