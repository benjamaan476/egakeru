
#pragma once
#include "pch.h"
#include "resource_loader.h"
#include "resources/geometry.h"

#include "platform/filesystem.h"

namespace egkr
{
	enum class mesh_file_type
	{
		obj,
		esm,
		not_found
	};

	struct supported_file_type
	{
		std::string extension{};
		mesh_file_type file_type{};
		bool is_binary{};
	};

	struct mesh_vertex_index_data
	{
		uint32_t position_index{};
		uint32_t normal_index{};
		uint32_t tex_index{};
	};

	struct mesh_face_data
	{
		std::array<mesh_vertex_index_data, 3> vertices{};
	};

	struct mesh_group_data
	{
		egkr::vector<mesh_face_data> faces{};
	};

	class mesh_loader : public resource_loader
	{
	public:
		using unique_ptr = std::unique_ptr<mesh_loader>;
		static unique_ptr create(const loader_properties& properties);

		explicit mesh_loader(const loader_properties& properties);
		~mesh_loader() override = default;

		resource::shared_ptr load(std::string_view name, void* params) override;
		bool unload(const resource::shared_ptr& resource) override;

	private:
		egkr::vector<geometry::properties> import_obj(file_handle& file_handle, std::string_view esm_filename);
		geometry::properties process_subobject(egkr::vector<float3>& positions, const egkr::vector<float3>& normals, const egkr::vector<float2>& tex, egkr::vector<mesh_face_data> faces);
		bool import_obj_material_library(std::string_view filepath);

		egkr::vector<geometry::properties> load_esm(file_handle& file_handle);
		bool write_esm(std::string_view path, const egkr::vector<geometry::properties>& properties);
		bool write_emt(std::string_view directory, const material_properties& properties);
	};
}
