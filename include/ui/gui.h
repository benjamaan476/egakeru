#pragma once

#include "../EngineCore.h"
#include "../renderer/Texture.h"
#include "../renderer/CommandBuffer.h"

class GuiImpl;

// Helper to check if a class is a vector
template<typename T, typename = void>
struct is_vector : std::false_type {};

template<typename T>
struct is_vector<T, std::void_t<typename T::value_type>> : std::true_type {};

template<typename T>
concept is_vector_v = is_vector<T, std::void_t<typename T::value_type>>::value;

template<typename Type, typename V>
concept is_vector_type = std::is_same<Type, typename V::value_type>::value;

class Gui
{
public:
	using SharedPtr = std::shared_ptr<Gui>;

	struct DropdownValue
	{
		uint32_t index{};
		std::string label{};
	};

	using DropdownList = std::vector<DropdownValue>;

	struct RadioButton
	{
		uint32_t id{};
		std::string label{};
		bool sameLine{};
	};

	using RadioButtonGroup = std::vector<RadioButton>;

	enum class TextFlags
	{
		Empty = 0x0,
		FitWindow = 0x1
	};

	enum class WindowFlags
	{
		Empty = 0x0,
		ShowTitleBar = 0x1,
		AllowMove = 0x2,
		SetFocus = 0x4,
		CloseButton = 0x8,
		NoResize = 0x10,
		AutoResize = 0x20,

		Default = ShowTitleBar | AllowMove | SetFocus | CloseButton
	};

	enum class WidgetFlags
	{
		Empty = 0x0,
		SameLine = 0x1,
		Inactive = 0x2,
	};

	enum class DragDropFlags
	{
		Empty = 0x0,
		Extern = 0x1,
	};

	class Group;

	class Widget
	{
	public:
		Group group(std::string_view label, bool beginExpanded = false);

		void indent(float i);
		void separator(uint32_t count = 1);
		void dummy(std::string_view label, const float2& size, bool sameLine = false);
		void rect(const float2& size, const float4& colour = { 1.f, 1.f, 1.f, 1.f }, bool filled = false, bool sameLine = false);
		bool dropdown(std::string_view label, const DropdownList& values, uint32_t& var, bool sameLine = false);
		bool button(std::string_view label, bool sameLine = false);
		bool radioButtons(const RadioButtonGroup& buttons, uint32_t& activeId);
		bool direction(std::string_view label, float3& direction);
		template<typename T>
		bool checkbox(std::string_view label, T& var, bool sameLine = false);

		bool dragDropSource(std::string_view label, std::string_view dataLabel, std::string_view payload);
		bool dragDropSource(std::string_view label, std::string_view dataLabel, std::string_view payload, DragDropFlags flags = DragDropFlags::Empty);
		void endDropSource();
		bool dragDropDestination(std::string_view dataLabel, std::string& payload);
		void endDropDestination();
		void text(std::string_view text, bool sameLine = false);
		void textWrapped(std::string_view text);
		bool textbox(std::string_view label, std::string& text, TextFlags flags = TextFlags::Empty);
		bool textboxMultiline(std::string_view label, const std::vector<std::string>& textLabels, std::vector<std::string>& textEntries);
		void tooltip(std::string_view text, bool sameLine = false);

		bool rgbColour(std::string_view label, float3& var, bool sameLine = false);
		bool rgbaColour(std::string_view label, float4& var, bool sameLine = false);

		void image(std::string_view label, const egkr::Texture2D& image, vk::Sampler sampler, float2 size = float2{}, bool maintainRatio = true, bool sameLine = false);
		void imageButton(std::string_view label, const egkr::Texture2D& image, vk::Sampler sampler, float2 size, bool maintainRatio = true, bool sameLine = false);


		template<typename Vec, typename Type>
		bool var(std::string_view label, Vec& var, Type minValue = std::numeric_limits<Type>::lowest(), Type maxValue = std::numeric_limits<Type>::max(), Type step = std::is_floating_point<Type>::value ? 0.001f : 1, bool sameLine = false);

		template<typename Vec, typename Type>
		bool slider(std::string_view label, Vec& var, Type minValue = std::numeric_limits<Type>::lowest() / 2, Type maxValue = std::numeric_limits<Type>::max() / 2, bool sameLine = false);

		template<typename MatrixType>
		bool matrix(std::string_view label, MatrixType& var, float minValue = -FLT_MAX, float maxValue = FLT_MAX, bool sameLine = false);


		Gui* gui() const { return _gui; }

	protected:
		Widget() = default;
		Gui* _gui{};
	};

	class Group : public Widget
	{
	public:
		Group() = default;
		Group(Gui* gui, std::string_view label, bool beginExpanded = false);
		Group(const Widget& widget, const std::string label, bool beginExpanded = false);

		bool isOpen() const;
		operator bool() const { return isOpen(); }

		~Group();

		void release();
	};

	class Window : public Widget
	{
	public:
		Window(Gui* gui, std::string_view name, uint2 size = { 0, 0 }, uint2 position = { 0, 0 }, WindowFlags flags = WindowFlags::Default);
		Window(Gui* gui, std::string_view name, bool& open, uint2 size = { 0, 0 }, uint2 position = { 0, 0 }, WindowFlags flags = WindowFlags::Default);
		Window(const Widget& widget, std::string_view name, uint2 size = { 0, 0 }, uint2 position = { 0, 0 }, WindowFlags flags = WindowFlags::Default);
		Window(const Widget& widget, std::string_view name, bool& open, uint2 size = { 0, 0 }, uint2 position = { 0, 0 }, WindowFlags flags = WindowFlags::Default);

		~Window();

		void release();
		void columns(uint32_t numColumns);
		void nextColumn();
		void windowPosition(uint32_t x, uint32_t y);
		void windowSize(uint32_t x, uint32_t y);
	};
	static SharedPtr create(float scaleFactor = 1.f);
	static SharedPtr get() 
	{ 
		if (_instance)
		{
			return _instance;
		}
		return create();
	}

	static void release();
	~Gui();

	static float4 pickUniqueColour(std::string_view key);

	//void addFont(std::string_view name, const std::filesystem::path& path);

	//void setActiveFont(std::string_view font);

	//ImFont* getFont(std::string f = "");

	void begin();
	void demo(bool showDemo);
	void render(vk::Extent2D extent, uint32_t currentFrame, uint32_t imageIndex);
	const auto getCommandBuffer(uint32_t currentFrame) const noexcept { return _uiCommandBuffers[currentFrame]; }
	static void setGlobalScaling(float scale);
	void onWindowResize(uint32_t width, uint32_t height, const std::vector<egkr::Texture2D>& _swapchainImages);

private:
	Gui() = default;
	GuiImpl* _wrapper = nullptr;
	static inline SharedPtr _instance;

	static inline vk::RenderPass _uiRenderPass{};
	static inline vk::DescriptorPool _uiPool{};
	static inline std::vector<vk::Framebuffer> _uiFramebuffers{};
	static inline egkr::CommandBuffer _uiCommandBuffers{};
};

ENUM_CLASS_OPERATORS(Gui::WindowFlags)
ENUM_CLASS_OPERATORS(Gui::TextFlags)
ENUM_CLASS_OPERATORS(Gui::DragDropFlags)
