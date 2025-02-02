#pragma once

#include "resource_loader.h"
#include "resources/terrain.h"

namespace egkr::loader
{
    class terrain : public resource_loader
    {
    public:
	using unique_ptr = std::unique_ptr<terrain>;
	static unique_ptr create(const loader_properties& properties);

	explicit terrain(const loader_properties& properties);
	terrain(const terrain&) = delete;
	terrain operator=(const terrain&) = delete;
	terrain(terrain&&) = delete;
	terrain operator=(terrain&&) = delete;

	~terrain() override = default;

	resource::shared_ptr load(const std::string& name, void* params) override;
	bool unload(const resource::shared_ptr& resource) override;
    private:
	egkr::terrain::configuration load_configuration_file(const std::string& path);
    };
}
