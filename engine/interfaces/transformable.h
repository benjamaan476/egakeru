//#pragma once
//
//#include "resources/transform.h"
//
//namespace egkr
//{
//	struct transformable_o
//	{
//		void set_transform(const transformable& transform) { transform_ = transform; }
//		[[nodiscard]] const auto& get_transform() const { return transform_; }
//		[[nodiscard]] auto& get_transform() { return transform_; }
//
//		[[nodiscard]] auto get_world_transform() const
//		{
//			transformable xform = transform_;
//			return xform.get_world(); 
//		}
//
//		[[nodiscard]] auto get_local_transform() const
//		{
//			transformable xform = transform_;
//			return xform.get_local(); 
//		}
//
//		void set_position(const float3& position)
//		{
//			transform_.set_position(position);
//		}
//		[[nodiscard]] const auto& get_position() const { return transform_.get_position(); }
//
//		void set_parent(transformable* parent)
//		{
//			transform_.set_parent(parent ? &parent->transform_ : nullptr);
//		}
//
//	protected:
//		transformable transform_{};
//	};
//}
