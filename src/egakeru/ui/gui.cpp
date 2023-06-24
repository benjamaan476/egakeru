#include "ui/gui.h"
#include "Application.h"

#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <imgui_impl_glfw.h>

#include <span>

#include <glm/gtc/type_ptr.hpp>

#include <map>

class GuiImpl
{
public:
	GuiImpl() = default;

private:
	friend class Gui;
	void init(float scaleFactor);
	void compileFonts();

	bool addCheckboxes(std::string_view label, std::span<bool> data, bool sameLine = false);

	struct ComboData
	{
		uint32_t lastValue{};
		int32_t currentItem{};
	};

	std::unordered_map<std::string, ComboData> _dropDownValues;

	std::map<vk::Image, VkDescriptorSet> _imageDescriptors;

	float _scaleFactor = 1.f;
	uint32_t _groupStackSize{};

	bool pushWindow(std::string_view name, bool& open, uint2 size = { 0, 0 }, uint2 position = { 0, 0 }, Gui::WindowFlags flags = Gui::WindowFlags::Default);
	void popWindow();
	void setCurrentWindowSize(uint32_t width, uint32_t height);
	void setCurrentWindowPosition(uint32_t x, uint32_t y);
	void beginColumns(uint32_t numColumns);
	void nextColumn();

	bool beginGroup(std::string_view label, bool beginExpanded = false);
	void endGroup();

	void indent(float i);
	void addSeparator(uint32_t count = 1);
	void addDummyItem(std::string_view label, const float2& size, bool sameLine = false);
	void addRect(const float2& size, const float4& colour = float4(1.0f, 1.0f, 1.0f, 1.0f), bool filled = false, bool sameLine = false);
	bool addDropdown(std::string_view label, const Gui::DropdownList& values, uint32_t& var, bool sameLine = false);
	bool addButton(std::string_view label, bool sameLine = false);
	bool addRadioButtons(const Gui::RadioButtonGroup& buttons, uint32_t& activeID);
	bool addDirection(std::string_view label, float3& direction);
	bool addCheckbox(std::string_view label, bool& var, bool sameLine = false);
	bool addCheckbox(std::string_view label, int& var, bool sameLine = false);
	
	template<typename T>
	bool addBoolVecVar(std::string_view label, T& var, bool sameLine = false);
	
	bool addDragDropSource(std::string_view label, std::string_view dataLabel, std::string_view payloadString, Gui::DragDropFlags flags = Gui::DragDropFlags::Empty);
	void endDragDropSource();
	bool addDragDropDest(std::string_view dataLabel, std::string& payloadString);
	void endDragDropDest();

	void addText(std::string_view text, bool sameLine = false);
	void addTextWrapped(std::string_view text);
	bool addTextbox(std::string_view label, std::string& text, uint32_t lineCount = 1, Gui::TextFlags flags = Gui::TextFlags::Empty);
	bool addMultiTextbox(std::string_view label, const std::vector<std::string>& textLabels, std::vector<std::string>& textEntries);
	void addTooltip(std::string_view tip, bool sameLine = true);

	bool addRgbColor(std::string_view label, float3& var, bool sameLine = false);
	bool addRgbaColor(std::string_view label, float4& var, bool sameLine = false);

	void addImage(std::string_view label, const egkr::Texture2D& image, vk::Sampler sampler, float2 size = float2{}, bool maintainRatio = true, bool sameLine = false);
	bool addImageButton(std::string_view label, const egkr::Texture2D& image, vk::Sampler sampler, float2 size, bool maintainRatio = true, bool sameLine = false);

	template<typename T>
	bool addScalarVar(std::string_view label, T& var, T minVal = std::numeric_limits<T>::lowest(), T maxVal = std::numeric_limits<T>::max(), T step = 1.0f, bool sameLine = false, const char* displayFormat = nullptr);
	template<typename T>
	bool addScalarSlider(std::string_view label, T& var, T minVal = std::numeric_limits<T>::lowest(), T maxVal = std::numeric_limits<T>::max(), bool sameLine = false, const char* displayFormat = nullptr);

	template<typename T>
	bool addVecVar(std::string_view label, T& var, typename T::value_type minVal = std::numeric_limits<typename T::value_type>::lowest(), typename T::value_type maxVal = std::numeric_limits<typename T::value_type>::max(), typename T::value_type step = 1.0f, bool sameLine = false, const char* displayFormat = nullptr);
	template<typename T>
	bool addVecSlider(std::string_view label, T& var, typename T::value_type minVal = std::numeric_limits<typename T::value_type>::lowest(), typename T::value_type maxVal = std::numeric_limits<typename T::value_type>::max(), bool sameLine = false, const char* displayFormat = nullptr);

	template<int R, int C, typename T>
	void addMatrixVar(std::string_view label, glm::mat<C, R, T>& var, float minValue = std::numeric_limits<float>::lowest(), float maxValue = std::numeric_limits<float>::max(), bool sameLine = false);
};

void GuiImpl::init(float scaleFactor)
{
	_scaleFactor = scaleFactor;
	ImGui::CreateContext();
	auto& io = ImGui::GetIO();

	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	ImGui::StyleColorsDark();
}

void GuiImpl::compileFonts()
{

}

bool GuiImpl::addCheckboxes(std::string_view label, std::span<bool> data, bool sameLine)
{
	auto modified = false;
	std::string labelString{ std::string("##") + label.data() + '0'};
	
	for (size_t i = 0; i < data.size() - 1; i++)
	{
		labelString[labelString.size() - 1] = '0' + static_cast<char>(i);
		modified |= addCheckbox(labelString, data[i], !i ? sameLine : true);
	}

	addCheckbox(label, data[data.size() - 1], true);

	return modified;
}

bool GuiImpl::pushWindow(std::string_view label, bool& open, uint2 size, uint2 position, Gui::WindowFlags flags)
{
	bool allowClose = isSet(flags, Gui::WindowFlags::CloseButton);
	if (allowClose)
	{
		if (!isSet(flags, Gui::WindowFlags::ShowTitleBar))
		{
			LOG_WARN("{}", "Asking for a close button on a window without a title bar");
		}
	}

	float2 posFloat{ position};
	posFloat *= _scaleFactor;

	ImGui::SetNextWindowSize({ (float)size.x, (float)size.y }, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowPos({ posFloat.x, posFloat.y }, ImGuiCond_FirstUseEver);

	auto imguiFlags = 0;

	if (!isSet(flags, Gui::WindowFlags::ShowTitleBar))
	{
		imguiFlags |= ImGuiWindowFlags_NoTitleBar;
	}
	if (!isSet(flags, Gui::WindowFlags::AllowMove))
	{
		imguiFlags |= ImGuiWindowFlags_NoMove;
	}
	if (!isSet(flags, Gui::WindowFlags::SetFocus))
	{
		imguiFlags |= ImGuiWindowFlags_NoFocusOnAppearing;
	}
	if (isSet(flags, Gui::WindowFlags::NoResize))
	{
		imguiFlags |= ImGuiWindowFlags_NoResize;
	}
	if (isSet(flags, Gui::WindowFlags::AutoResize))
	{
		imguiFlags |= ImGuiWindowFlags_AlwaysAutoResize;
	}

	ImGui::Begin(label.data(), allowClose ? &open : nullptr, imguiFlags);

	if (!open)
	{
		ImGui::End();
	}

	return open;
}

void GuiImpl::popWindow()
{
	ImGui::End();
}

void GuiImpl::setCurrentWindowPosition(uint32_t x, uint32_t y)
{
	ImGui::SetWindowPos({(float)x, (float)y});
}

void GuiImpl::setCurrentWindowSize(uint32_t width, uint32_t height)
{
	ImGui::SetWindowPos({(float)width, (float)height});
}

void GuiImpl::beginColumns(uint32_t numColumns)
{
	ImGui::Columns(numColumns);
}

void GuiImpl::nextColumn()
{
	ImGui::NextColumn();
}

bool GuiImpl::beginGroup(std::string_view name, bool beginExpanded)
{
	auto flags = beginExpanded ? ImGuiTreeNodeFlags_DefaultOpen : 0;
	auto visible = _groupStackSize ? ImGui::TreeNodeEx(name.data(), flags) : ImGui::CollapsingHeader(name.data(), flags);
	if (visible)
	{
		_groupStackSize++;
	}

	return visible;
}

void GuiImpl::endGroup()
{
	ENGINE_ASSERT(_groupStackSize >= 1, "Attempting to end a group that wasn't begun");
	_groupStackSize--;
	if (_groupStackSize)
	{
		ImGui::TreePop();
	}
}

void GuiImpl::indent(float i)
{
	ImGui::Indent(i);
}

void GuiImpl::addSeparator(uint32_t count)
{
	for (uint32_t i = 0; i < count; i++)
	{
		ImGui::Separator();
	}
}

void GuiImpl::addDummyItem(std::string_view label, const float2& size, bool sameLine)
{
	if (sameLine)
	{
		ImGui::SameLine();
	}

	ImGui::PushID(label.data());
	ImGui::Dummy({ size.x, size.y });
	ImGui::PopID();
}

void GuiImpl::addRect(const float2& size, const float4& colour, bool filled, bool sameLine)
{
	if (sameLine)
	{
		ImGui::SameLine();
	}

	const auto& cursorPosition = ImGui::GetCursorScreenPos();

	ImVec2 bottomLeft{ cursorPosition.x + size.x, cursorPosition.y + size.y };
	ImVec4 rectColour{ colour.r, colour.g, colour.b, colour.a };

	if (filled)
	{
		ImGui::GetWindowDrawList()->AddRectFilled(cursorPosition, bottomLeft, ImGui::ColorConvertFloat4ToU32(rectColour));
	}
	else
	{
		ImGui::GetWindowDrawList()->AddRect(cursorPosition, bottomLeft, ImGui::ColorConvertFloat4ToU32(rectColour));
	}
}

bool GuiImpl::addDropdown(std::string_view label, const Gui::DropdownList& values, uint32_t& var, bool sameLine)
{
	if (values.empty())
	{
		return false;
	}

	if (sameLine)
	{
		ImGui::SameLine();
	}

	const auto& iter = _dropDownValues.find(label.data());

	auto currentItem = -1;

	if ((iter == _dropDownValues.end()) || (iter->second.lastValue != var))
	{
		for (auto i = 0; i < values.size(); i++)
		{
			if (values[i].index == var)
			{
				currentItem = i;
				_dropDownValues[label.data()].currentItem = i;
				break;
			}
		}
	}
	else
	{
		currentItem = _dropDownValues[label.data()].currentItem;
	}

	std::string comboString;
	for (const auto& v : values)
	{
		comboString += v.label + '\0';
	}
	comboString += '\0';

	auto previousItem = currentItem;

	auto b = ImGui::Combo(label.data(), &currentItem, comboString.data());
	_dropDownValues[label.data()].currentItem = currentItem;
	_dropDownValues[label.data()].lastValue = values[currentItem].index;
	var = values[currentItem].index;

	return b && previousItem != currentItem;
}

bool GuiImpl::addButton(std::string_view label, bool sameLine)
{
	if (sameLine)
	{
		ImGui::SameLine();
	}
	return ImGui::Button(label.data());
}

bool GuiImpl::addRadioButtons(const Gui::RadioButtonGroup& buttons, uint32_t& activeID)
{
	auto oldValue = activeID;

	for (const auto& button : buttons)
	{
		if (button.sameLine)
		{
			ImGui::SameLine();
		}
		ImGui::RadioButton(button.label.data(), (int*)&activeID, button.id);
	}

	return oldValue != activeID;
}

bool GuiImpl::addDirection(std::string_view label, float3& direction)
{
	auto dir = glm::normalize(direction);
	auto b = addVecVar(label, dir, -1.f, 1.f, 0.001f, false, "%.3f");

	if (b)
	{
		direction = glm::normalize(dir);
	}

	return b;
}

bool GuiImpl::addCheckbox(std::string_view label, bool& var, bool sameLine)
{
	if (sameLine)
	{
		ImGui::SameLine();
	}
	return ImGui::Checkbox(label.data(), &var);
}

bool GuiImpl::addCheckbox(std::string_view label, int& var, bool sameLine)
{
	auto value = var != 0;
	auto modified = addCheckbox(label, value, sameLine);

	var = value ? 1 : 0;
	return modified;
}

template<typename T>
bool GuiImpl::addBoolVecVar(std::string_view label, T& var, bool sameLine)
{
	return addCheckboxes(label, { glm::value_ptr(var), var.length() }, sameLine);
}

bool GuiImpl::addDragDropSource(std::string_view, std::string_view dataLabel, std::string_view payloadString, Gui::DragDropFlags flags)
{
	if (ImGui::IsItemHovered() && (ImGui::IsMouseClicked(0) || ImGui::IsMouseClicked(1)))
	{
		ImGui::SetWindowFocus();
	}

	if (!(ImGui::IsWindowFocused()))
	{
		return false;
	}

	ImGuiDragDropFlags dragDropFlags = ImGuiDragDropFlags_SourceAllowNullID;
	if (isSet(flags, Gui::DragDropFlags::Extern))
	{
		dragDropFlags |= ImGuiDragDropFlags_SourceExtern;
	}

	auto b = ImGui::BeginDragDropSource(dragDropFlags);

	if (b)
	{
		ImGui::SetDragDropPayload(dataLabel.data(), payloadString.data(), payloadString.size() * sizeof(payloadString[0]), ImGuiCond_Always);

	}

	return b;
}

void GuiImpl::endDragDropSource()
{
	ImGui::EndDragDropSource();
}

bool GuiImpl::addDragDropDest(std::string_view dataLabel, std::string& payloadString)
{
	auto b = false;

	if (ImGui::BeginDragDropTarget())
	{
		auto dragDropPayload = ImGui::AcceptDragDropPayload(dataLabel.data());

		b = dragDropPayload && dragDropPayload->IsDataType(dataLabel.data()) && (dragDropPayload->Data != nullptr);

		if (b)
		{
			payloadString.resize(dragDropPayload->DataSize);
			std::memcpy(&payloadString.front(), dragDropPayload->Data, dragDropPayload->DataSize);
		}

	}

	return b;
}

void GuiImpl::endDragDropDest()
{
	ImGui::EndDragDropTarget();
}

void GuiImpl::addText(std::string_view text, bool sameLine)
{
	if (sameLine)
	{
		ImGui::SameLine();
	}
	ImGui::TextUnformatted(text.data());
}

void GuiImpl::addTextWrapped(std::string_view text)
{
	ImGui::TextWrapped("%s", text.data());
}

bool GuiImpl::addTextbox(std::string_view label, std::string& text, uint32_t lineCount, Gui::TextFlags flags)
{
	auto fitWindow = isSet(flags, Gui::TextFlags::FitWindow);

	if (fitWindow)
	{
		ImGui::PushItemWidth(ImGui::GetWindowWidth());
	}

	static const uint32_t maxSize = 2048;
	char buf[maxSize];
	const uint32_t length = std::min(maxSize - 1, (uint32_t)text.length());
	text.copy(buf, length);
	buf[length] = '\0';

	auto updated = false;
	if (lineCount > 1)
	{
		auto textFlags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CtrlEnterForNewLine;
		updated = ImGui::InputTextMultiline(label.data(), buf, length, { -1.f, ImGui::GetTextLineHeight() * lineCount }, textFlags);
	}
	else
	{
		updated = ImGui::InputText(label.data(), buf, length, ImGuiInputTextFlags_EnterReturnsTrue);
	}

	if (fitWindow)
	{
		ImGui::PopItemWidth();
	}
	return updated;
}


bool GuiImpl::addMultiTextbox(std::string_view label, const std::vector<std::string>& textLabels, std::vector<std::string>& textEntries)
{
	static uint32_t idOffset = 0;
	auto result = false;

	for (uint32_t i = 0; i < textEntries.size(); i++)
	{
		result |= addTextbox(std::string{ textLabels[i] + "##" + std::to_string(idOffset) }, textEntries[i]);
	}

	return addButton(label) || result;
}

void GuiImpl::addTooltip(std::string_view tip, bool sameLine)
{
	if (sameLine)
	{
		ImGui::SameLine();
	}
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(450.f);
		ImGui::TextUnformatted(tip.data());
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

bool GuiImpl::addRgbColor(std::string_view label, float3& var, bool sameLine)
{
	if (sameLine)
	{
		ImGui::SameLine();
	}

	return ImGui::ColorEdit3(label.data(), glm::value_ptr(var));
}

bool GuiImpl::addRgbaColor(std::string_view label, float4& var, bool sameLine)
{
	if (sameLine)
	{
		ImGui::SameLine();
	}

	return ImGui::ColorEdit4(label.data(), glm::value_ptr(var));
}

void GuiImpl::addImage(std::string_view label, const egkr::Texture2D& image, vk::Sampler sampler, float2 size, bool maintainRatio, bool sameLine)
{
	if (size == float2(0))
	{
		auto windowSize = ImGui::GetWindowSize();
		size = { windowSize.x, windowSize.y };
	}

	ImGui::PushID(label.data());
	if (sameLine)
	{
		ImGui::SameLine();
	}

	if (!_imageDescriptors.contains(image.image))
	{
		_imageDescriptors.insert({ image.image, ImGui_ImplVulkan_AddTexture(sampler, image.view, static_cast<VkImageLayout>(vk::ImageLayout::eShaderReadOnlyOptimal)) });
	}

	auto aspectRatio = maintainRatio ? (float)image.getHeight() / (float)image.getWidth() : 1.f;


	ImGui::Image(_imageDescriptors[image.image], {size.x, maintainRatio ? size.x * aspectRatio : size.y});

	ImGui::PopID();
}

bool GuiImpl::addImageButton(std::string_view, const egkr::Texture2D& image, vk::Sampler sampler, float2 size, bool maintainRatio, bool sameLine)
{
	if (sameLine)
	{
		ImGui::SameLine();
	}


	if (!_imageDescriptors.contains(image.image))
	{
		_imageDescriptors.insert({ image.image,ImGui_ImplVulkan_AddTexture(sampler, image.view, static_cast<VkImageLayout>(vk::ImageLayout::eShaderReadOnlyOptimal)) });
	}

	auto aspectRatio = maintainRatio ? (float)image.getHeight() / (float)image.getWidth() : 1.f;
	return ImGui::ImageButton(_imageDescriptors[image.image], { size.x, maintainRatio ? size.x * aspectRatio : size.y });
}


template<typename T>
bool addScalarVarHelper(std::string_view label, T& var, ImGuiDataType_ imguiType, T minValue, T maxValue, T step, bool sameLine, const char* displayFormat)
{
	ImGui::PushItemWidth(200);
	if (sameLine)
	{
		ImGui::SameLine();
	}
	auto b = ImGui::DragScalar(label.data(), imguiType, &var, (float)step, &minValue, &maxValue, displayFormat);
	var = glm::clamp(var, minValue, maxValue);
	ImGui::PopItemWidth();

	return b;
}

template<typename T>
struct foobar : std::false_type
{ };

template<typename T>
bool GuiImpl::addScalarVar(std::string_view label, T& var, T minVal, T maxVal, T step, bool sameLine, const char* displayFormat)
{
	if constexpr (std::is_same<T, int32_t>::value)
	{
		return addScalarVarHelper(label, var, ImGuiDataType_S32, minVal, maxVal, step, sameLine, displayFormat);
	}
	else if constexpr (std::is_same<T, uint32_t>::value)
	{
		return addScalarVarHelper(label, var, ImGuiDataType_U32, minVal, maxVal, step, sameLine, displayFormat);
	}
	else if constexpr (std::is_same<T, int64_t>::value)
	{
		return addScalarVarHelper(label, var, ImGuiDataType_S64, minVal, maxVal, step, sameLine, displayFormat);
	}
	else if constexpr (std::is_same<T, uint64_t>::value)
	{
		return addScalarVarHelper(label, var, ImGuiDataType_U64, minVal, maxVal, step, sameLine, displayFormat);
	}
	else if constexpr (std::is_same<T, float>::value)
	{
		return addScalarVarHelper(label, var, ImGuiDataType_Float, minVal, maxVal, step, sameLine, displayFormat);
	}
	else if constexpr (std::is_same<T, uint64_t>::value)
	{
		return addScalarVarHelper(label, var, ImGuiDataType_U64, minVal, maxVal, step, sameLine, displayFormat);
	}
	else if constexpr (std::is_same<T, double_t>::value)
	{
		return addScalarVarHelper(label, var, ImGuiDataType_Double, minVal, maxVal, step, sameLine, displayFormat);
	}
	else 
	{
		static_assert(foobar<T>::value, "Unsupported data type");
	}
}

template<typename T>
bool addScalarSliderHelper(std::string_view label, T& var, ImGuiDataType_ imguiType, T minValue, T maxValue, bool sameLine, const char* displayFormat)
{
	ImGui::PushItemWidth(200);
	if (sameLine)
	{
		ImGui::SameLine();
	}
	auto b = ImGui::SliderScalar(label.data(), imguiType, &var, &minValue, &maxValue, displayFormat);
	var = glm::clamp(var, minValue, maxValue);
	ImGui::PopItemWidth();

	return b;
}

template<typename T>
bool GuiImpl::addScalarSlider(std::string_view label, T& var, T minVal, T maxVal, bool sameLine, const char* displayFormat)
{
	if constexpr (std::is_same<T, int32_t>::value)
	{
		return addScalarSliderHelper(label, var, ImGuiDataType_S32, minVal, maxVal, sameLine, displayFormat);
	}
	else if constexpr (std::is_same<T, uint32_t>::value)
	{
		return addScalarSliderHelper(label, var, ImGuiDataType_U32, minVal, maxVal, sameLine, displayFormat);
	}
	else if constexpr (std::is_same<T, int64_t>::value)
	{
		return addScalarSliderHelper(label, var, ImGuiDataType_S64, minVal, maxVal, sameLine, displayFormat);
	}
	else if constexpr (std::is_same<T, uint64_t>::value)
	{
		return addScalarSliderHelper(label, var, ImGuiDataType_U64, minVal, maxVal, sameLine, displayFormat);
	}
	else if constexpr (std::is_same<T, float>::value)
	{
		return addScalarSliderHelper(label, var, ImGuiDataType_Float, minVal, maxVal, sameLine, displayFormat);
	}
	else if constexpr (std::is_same<T, uint64_t>::value)
	{
		return addScalarSliderHelper(label, var, ImGuiDataType_U64, minVal, maxVal, sameLine, displayFormat);
	}
	else if constexpr (std::is_same<T, double_t>::value)
	{
		return addScalarSliderHelper(label, var, ImGuiDataType_Double, minVal, maxVal, sameLine, displayFormat);
	}
	else
	{
		static_assert(foobar<T>::value, "Unsupported data type");
	}
}

template<typename T>
bool addVecVarHelper(std::string_view label, T& var, ImGuiDataType_ imguiType, typename T::value_type minValue, typename T::value_type maxValue, typename T::value_type step, bool sameLine, const char* displayFormat)
{
	ImGui::PushItemWidth(200);
	if (sameLine)
	{
		ImGui::SameLine();
	}
	auto b = ImGui::DragScalarN(label.data(), imguiType, glm::value_ptr(var), var.length(), (float)step, &minValue, &maxValue, displayFormat);
	var = glm::clamp(var, minValue, maxValue);
	ImGui::PopItemWidth();

	return b;
}


template<typename T>
bool GuiImpl::addVecVar(std::string_view label, T& var, typename T::value_type minVal, typename T::value_type maxVal, typename T::value_type step, bool sameLine, const char* displayFormat)
{
	if constexpr (std::is_same<typename T::value_type, int32_t>::value)
	{
		return addVecVarHelper(label, var, ImGuiDataType_S32, minVal, maxVal, step, sameLine, displayFormat);
	}
	else if constexpr (std::is_same<typename T::value_type, uint32_t>::value)
	{
		return addVecVarHelper(label, var, ImGuiDataType_U32, minVal, maxVal, step, sameLine, displayFormat);
	}
	else if constexpr (std::is_same<typename T::value_type, int64_t>::value)
	{
		return addVecVarHelper(label, var, ImGuiDataType_S64, minVal, maxVal, step, sameLine, displayFormat);
	}
	else if constexpr (std::is_same<typename T::value_type, uint64_t>::value)
	{
		return addVecVarHelper(label, var, ImGuiDataType_U64, minVal, maxVal, step, sameLine, displayFormat);
	}
	else if constexpr (std::is_same<typename T::value_type, float>::value)
	{
		return addVecVarHelper(label, var, ImGuiDataType_Float, minVal, maxVal, step, sameLine, displayFormat);
	}
	else if constexpr (std::is_same<typename T::value_type, uint64_t>::value)
	{
		return addVecVarHelper(label, var, ImGuiDataType_U64, minVal, maxVal, step, sameLine, displayFormat);
	}
	else
	{
		static_assert(foobar<T>::value, "Unsupported data type");
	}
}

template<typename T>
bool addVecSliderHelper(std::string_view label, T& var, ImGuiDataType_ imguiType, typename T::value_type minValue, typename T::value_type maxValue, bool sameLine, const char* displayFormat)
{
	ImGui::PushItemWidth(200);
	if (sameLine)
	{
		ImGui::SameLine();
	}
	auto b = ImGui::SliderScalarN(label.data(), imguiType, glm::value_ptr(var), var.length(), &minValue, &maxValue, displayFormat);
	var = glm::clamp(var, minValue, maxValue);
	ImGui::PopItemWidth();

	return b;
}

template<typename T>
bool GuiImpl::addVecSlider(std::string_view label, T& var, typename T::value_type minVal, typename T::value_type maxVal, bool sameLine, const char* displayFormat)
{
	if constexpr (std::is_same<typename T::value_type, int32_t>::value)
	{
		return addVecSliderHelper(label, var, ImGuiDataType_S32, minVal, maxVal, sameLine, displayFormat);
	}
	else if constexpr (std::is_same<typename T::value_type, uint32_t>::value)
	{
		return addVecSliderHelper(label, var, ImGuiDataType_U32, minVal, maxVal, sameLine, displayFormat);
	}
	else if constexpr (std::is_same<typename T::value_type, int64_t>::value)
	{
		return addVecSliderHelper(label, var, ImGuiDataType_S64, minVal, maxVal, sameLine, displayFormat);
	}
	else if constexpr (std::is_same<typename T::value_type, uint64_t>::value)
	{
		return addVecSliderHelper(label, var, ImGuiDataType_U64, minVal, maxVal, sameLine, displayFormat);
	}
	else if constexpr (std::is_same<typename T::value_type, float>::value)
	{
		return addVecSliderHelper(label, var, ImGuiDataType_Float, minVal, maxVal, sameLine, displayFormat);
	}
	else
	{
		static_assert(foobar<T>, "Unsupported data type");
	}
}

template<int R, int C, typename T>
void GuiImpl::addMatrixVar(std::string_view label, glm::mat<C, R, T>& var, float minValue, float maxValue, bool sameLine)
{
	std::string labelString{ label };
	std::string hiddenLabelString{ "##" };
	hiddenLabelString += labelString + "[0]";

	auto topLeft = ImGui::GetCursorScreenPos();
	ImVec2 bottomRight;

	auto b = false;

	for (uint32_t i = 0; i < var.length(); i++)
	{
		auto stringToDisplay = hiddenLabelString;
		hiddenLabelString[hiddenLabelString.size() - 2] = '0' + i;

		if (i == var.length() - 1)
		{
			stringToDisplay = labelString;
		}

		b |= addVecVar<C>(stringToDisplay, var[i], minValue, maxValue, 0.001f, sameLine);

		if (i == 0)
		{
			ImGui::SameLine();
			bottomRight = ImGui::GetCursorScreenPos();
			auto oldSpace = ImGui::GetStyle().ItemSpacing.y;

			ImGui::GetStyle().ItemSpacing.y = 0;
			ImGui::Dummy({});
			ImGui::Dummy({});
			ImGui::GetStyle().ItemSpacing.y = oldSpace;

			auto correctedCursorPosition = ImGui::GetCursorScreenPos();
			correctedCursorPosition.y += oldSpace;
			ImGui::GetCursorScreenPos(correctedCursorPosition);
			bottomRight.y = ImGui::GetCursorScreenPos().y;
		}
		else if (i == 1)
		{
			bottomRight.y = topLeft.y + (bottomRight.y - topLeft.y) * (var.length());
			bottomRight.x -= ImGui::GetStyle().ItemInnerSpacing.x * 3 - 1;
			bottomRight.y -= ImGui::GetStyle().ItemInnerSpacing.y - 1;
			topLeft.x -= 1; topLeft.y -= 1;
			auto colourVec4 = ImGui::GetStyleColorVec4(ImGuiCol_ScrollbarGrab); 
			colourVec4.w *= 0.25f;
			auto colour = ImGui::ColorConvertFloat4ToU32(colourVec4);
			ImGui::GetWindowDrawList()->AddRect(topLeft, bottomRight, colour);
		}
	}

	return b;
}

void Gui::release()
{

	{
		if (_instance)
		{
			ImGui_ImplVulkan_Shutdown();
			ImGui_ImplGlfw_Shutdown();

			for (auto& framebuffer : _uiFramebuffers)
			{
				egkr::state.device.destroyFramebuffer(framebuffer);
			}

			egkr::state.device.destroyDescriptorPool(_uiPool);
			egkr::state.device.destroyRenderPass(_uiRenderPass);

			ImGui::DestroyContext();
			_instance.reset();
		}
	}
}

Gui::~Gui()
{
	if (_instance)
	{
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();

		for (auto& framebuffer : _uiFramebuffers)
		{
			egkr::state.device.destroyFramebuffer(framebuffer);
		}

		egkr::state.device.destroyDescriptorPool(_uiPool);
		egkr::state.device.destroyRenderPass(_uiRenderPass);

		ImGui::DestroyContext();
		_instance.reset();
	}
}

Gui::SharedPtr Gui::create(float scaleFactor)
{
	if (_instance)
	{
		return _instance;
	}

	const auto& renderer = egkr::egakeru::get();

	_instance = std::unique_ptr<Gui>(new Gui);
	_instance->_wrapper = new GuiImpl;
	_instance->_wrapper->init(scaleFactor);

	auto& app = Application::get();
	auto& window = app.window();

	vk::AttachmentDescription imguiAttachment{};
	imguiAttachment
		.setFormat(renderer.getFormat())
		.setSamples(vk::SampleCountFlagBits::e1)
		.setLoadOp(vk::AttachmentLoadOp::eLoad)
		.setStoreOp(vk::AttachmentStoreOp::eStore)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setInitialLayout(vk::ImageLayout::eColorAttachmentOptimal)
		.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

	vk::AttachmentReference imguiColourAttachment{};
	imguiColourAttachment
		.setAttachment(0)
		.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

	vk::SubpassDescription imguiSupass{};
	imguiSupass
		.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
		.setColorAttachments(imguiColourAttachment);

	vk::SubpassDependency imguiSubpassDependency{};
	imguiSubpassDependency
		.setSrcSubpass(VK_SUBPASS_EXTERNAL)
		.setDstSubpass(0)
		.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		//.setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
		.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);

	vk::RenderPassCreateInfo imguiInfo{};
	imguiInfo
		.setAttachments(imguiAttachment)
		.setSubpasses(imguiSupass)
		.setDependencies(imguiSubpassDependency);

	_uiRenderPass = egkr::state.device.createRenderPass(imguiInfo);
	ENGINE_ASSERT(_uiRenderPass != vk::RenderPass{}, "Failed to create imgui render pass");

	const auto& swapchainImages = renderer.getSwapchainImages();

	_uiCommandBuffers = egkr::CommandBuffer(swapchainImages.size());

	const uint32_t DescriptorCount = 1000;

#define DESCRIPTOR_POOL(name, type)			\
	vk::DescriptorPoolSize name{};			\
	name									\
		.setType(type) 						\
		.setDescriptorCount(DescriptorCount)


	DESCRIPTOR_POOL(sampler, vk::DescriptorType::eSampler);
	DESCRIPTOR_POOL(combinedSample, vk::DescriptorType::eCombinedImageSampler);
	DESCRIPTOR_POOL(sampled, vk::DescriptorType::eSampledImage);
	DESCRIPTOR_POOL(storageImage, vk::DescriptorType::eStorageImage);
	DESCRIPTOR_POOL(uniformTexel, vk::DescriptorType::eUniformTexelBuffer);
	DESCRIPTOR_POOL(storageTexel, vk::DescriptorType::eStorageTexelBuffer);
	DESCRIPTOR_POOL(uniform, vk::DescriptorType::eUniformBuffer);
	DESCRIPTOR_POOL(storage, vk::DescriptorType::eStorageBuffer);
	DESCRIPTOR_POOL(uniformDynamic, vk::DescriptorType::eUniformBufferDynamic);
	DESCRIPTOR_POOL(storageDynamic, vk::DescriptorType::eStorageBufferDynamic);
	DESCRIPTOR_POOL(input, vk::DescriptorType::eInputAttachment);


	auto pools = { sampler, combinedSample, sampled, storageImage, uniformTexel, storageTexel, uniform, storage, uniformDynamic, storageDynamic, input };

	vk::DescriptorPoolCreateInfo poolInfo{};
	poolInfo
		.setPoolSizes(pools)
		.setMaxSets(DescriptorCount * (uint32_t)pools.size())
		.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);

	_uiPool = egkr::state.device.createDescriptorPool(poolInfo);
	ENGINE_ASSERT(_uiPool != vk::DescriptorPool{}, "Failed to create desriptor pool")

	ImGui_ImplGlfw_InitForVulkan(window->getWindow(), true);


	ImGui_ImplVulkan_InitInfo initInfo{};
	initInfo.Instance = egkr::state.instance;
	initInfo.PhysicalDevice = egkr::state.physicalDevice;
	initInfo.Device = egkr::state.device;
	initInfo.Queue = egkr::state.graphicsQueue;
	initInfo.QueueFamily = egkr::state.queueFamily;
	initInfo.DescriptorPool = _uiPool;
	initInfo.Subpass = 0;
	initInfo.MinImageCount = 2;
	initInfo.ImageCount = (uint32_t)swapchainImages.size();
	initInfo.Allocator = nullptr;
	initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	initInfo.CheckVkResultFn = [](VkResult err)
	{
		if (err == 0)
			return;
		fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
		if (err < 0)
			abort();
	};


	auto success = ImGui_ImplVulkan_Init(&initInfo, _uiRenderPass);

	ENGINE_ASSERT(success, "Failed to initialise vulkan for imgui");

	egkr::state.device.resetCommandPool(egkr::state.commandPool);

	egkr::OneTimeCommandBuffer command{
		[](vk::CommandBuffer buffer)
		{
			ImGui_ImplVulkan_CreateFontsTexture(buffer);
		} };

	ImGui_ImplVulkan_DestroyFontUploadObjects();

	auto [width, height] = window->getFramebufferSize();

	_instance->onWindowResize(width, height, swapchainImages);
	return _instance;
}

float4 Gui::pickUniqueColour(std::string_view key)
{
	union hashedValue
	{
		size_t t;
		int32_t i[2];
	} colour{};

	colour.t = std::hash<const char*>()(key.data());

	return float4(colour.i[0] % 1000 / 2000.f, colour.i[1] % 1000 / 2000.f, (colour.i[0] * colour.i[1]) % 1000 / 2000.f, 1.f);
}

void Gui::begin()
{
	// Start the Dear ImGui frame
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();

	ImGui::NewFrame();
}

void Gui::demo(bool showDemo)
{
	if (showDemo)
	{
		ImGui::ShowDemoWindow(&showDemo);
	}
}

void Gui::setGlobalScaling(float scale)
{
	auto& io = ImGui::GetIO();
	io.FontGlobalScale = scale;
	ImGui::GetStyle().ScaleAllSizes(scale);
}

void Gui::render(vk::Extent2D extent, uint32_t currentFrame, uint32_t imageIndex)
{
	while (_wrapper->_groupStackSize)
	{
		_wrapper->endGroup();
	}

	ImGui::Render();

	_uiCommandBuffers.record(currentFrame, imageIndex,
		[&](vk::CommandBuffer commandBuffer, uint32_t imageIndex)
		{
			vk::ClearValue clearColour;
			clearColour.color.setFloat32({ 0.f, 0.f, 0.f, 1.f });

			auto clearValues = { clearColour };

			vk::RenderPassBeginInfo imguiRenderPassInfo{};
			imguiRenderPassInfo
				.setRenderPass(_uiRenderPass)
				.setFramebuffer(_uiFramebuffers[imageIndex])
				.setRenderArea(vk::Rect2D({ 0, 0 }, extent))
				.setClearValues(clearValues);

			commandBuffer.beginRenderPass(imguiRenderPassInfo, vk::SubpassContents::eInline);


			auto drawData = ImGui::GetDrawData();
			ImGui_ImplVulkan_RenderDrawData(drawData, commandBuffer);

			commandBuffer.endRenderPass();
		});

	_wrapper->_groupStackSize = 0;

	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
}

void Gui::onWindowResize(uint32_t width, uint32_t height, const std::vector<egkr::Texture2D>& _swapchainImages)
{
	auto& io = ImGui::GetIO();
	io.DisplaySize.x = (float)width;
	io.DisplaySize.y = (float)height;

	for (auto& framebuffer : _uiFramebuffers)
	{
		egkr::state.device.destroyFramebuffer(framebuffer);
	}

	_uiFramebuffers.resize(_swapchainImages.size());

	for (auto i = 0; i < _swapchainImages.size(); i++)
	{
		vk::FramebufferCreateInfo imguiFrameBufferInfo{};
		imguiFrameBufferInfo
			.setRenderPass(_uiRenderPass)
			.setAttachments(_swapchainImages[i].view)
			.setWidth(width)
			.setHeight(height)
			.setLayers(1);

		_uiFramebuffers[i] = egkr::state.device.createFramebuffer(imguiFrameBufferInfo);
	}
}

Gui::Group Gui::Widget::group(std::string_view label, bool beginExpanded)
{
	return Group(_gui, label, beginExpanded);
}

void Gui::Widget::indent(float i)
{
	if (_gui)
	{
		_gui->_wrapper->indent(i);
	}
}

void Gui::Widget::separator(uint32_t count)
{
	if (_gui)
	{
		_gui->_wrapper->addSeparator(count);
	}
}

void Gui::Widget::dummy(std::string_view label, const float2& size, bool sameLine)
{
	if (_gui)
	{
		_gui->_wrapper->addDummyItem(label, size, sameLine);
	}
}

void Gui::Widget::rect(const float2& size, const float4& colour, bool filled, bool sameLine)
{
	if (_gui)
	{
		_gui->_wrapper->addRect(size, colour, filled, sameLine);
	}
}
bool Gui::Widget::dropdown(std::string_view label, const DropdownList& values, uint32_t& var, bool sameLine)
{
	return _gui ? _gui->_wrapper->addDropdown(label, values, var, sameLine) : false;
}

bool Gui::Widget::button(std::string_view label, bool sameLine)
{
	return _gui ? _gui->_wrapper->addButton(label, sameLine) : false;
}

bool Gui::Widget::radioButtons(const RadioButtonGroup& buttons, uint32_t& activeID)
{
	return _gui ? _gui->_wrapper->addRadioButtons(buttons, activeID) : false;
}

bool Gui::Widget::direction(std::string_view label, float3& direction)
{
	return _gui ? _gui->_wrapper->addDirection(label, direction) : false;
}

template<>
bool Gui::Widget::checkbox<bool>(std::string_view label, bool& var, bool sameLine)
{
	return _gui ? _gui->_wrapper->addCheckbox(label, var, sameLine) : false;
}

template<>
bool Gui::Widget::checkbox<int>(std::string_view label, int& var, bool sameLine)
{
	return _gui ? _gui->_wrapper->addCheckbox(label, var, sameLine) : false;
}


template<typename T>
bool Gui::Widget::checkbox(std::string_view label, T& var, bool sameLine)
{
	return _gui ? _gui->_wrapper->addBoolVecVar(label, var, sameLine) : false;
}

bool Gui::Widget::dragDropSource(std::string_view label, std::string_view dataLabel, std::string_view payloadString)
{
	return _gui ? _gui->_wrapper->addDragDropSource(label, dataLabel, payloadString) : false;
}

void Gui::Widget::endDropSource()
{
	if (_gui)
	{
		_gui->_wrapper->endDragDropSource();
	}
}

bool Gui::Widget::dragDropSource(std::string_view label, std::string_view dataLabel, std::string_view payloadString, DragDropFlags flags)
{
	return _gui ? _gui->_wrapper->addDragDropSource(label, dataLabel, payloadString, flags) : false;
}

bool Gui::Widget::dragDropDestination(std::string_view label, std::string& payloadString)
{
	return _gui ? _gui->_wrapper->addDragDropDest(label, payloadString) : false;
}

void Gui::Widget::endDropDestination()
{
	if (_gui)
	{
		_gui->_wrapper->endDragDropDest();
	}
}

template<typename Vec, typename Type>
bool Gui::Widget::var(std::string_view label, Vec& var, Type minValue, Type maxValue, Type step, bool sameLine)
{
	if constexpr (is_vector_v<Vec> && is_vector_type<Type, Vec>)
	{
		return _gui ? _gui->_wrapper->addVecVar(label, var, minValue, maxValue, step, sameLine) : false;
	}
	else
	{
		return _gui ? _gui->_wrapper->addScalarVar(label, var, minValue, maxValue, step, sameLine) : false;
	}
}

#define ADD_SCALAR_VAR_TYPE(TypeName) template bool Gui::Widget::var<TypeName>(std::string_view label, TypeName& var, TypeName minValue, TypeName maxValue, TypeName step, bool sameLine);

ADD_SCALAR_VAR_TYPE(int32_t)
ADD_SCALAR_VAR_TYPE(uint32_t)
ADD_SCALAR_VAR_TYPE(uint64_t)
ADD_SCALAR_VAR_TYPE(float_t)
ADD_SCALAR_VAR_TYPE(double_t)

#undef ADD_SCALAR_VAR_TYPE


#define ADD_VEC_VAR_TYPE(TypeName) template bool Gui::Widget::var<TypeName>(std::string_view label, TypeName& var, typename TypeName::value_type minValue, typename TypeName::value_type maxValue, typename TypeName::value_type step, bool sameLine);

ADD_VEC_VAR_TYPE(int2)
ADD_VEC_VAR_TYPE(float2)
ADD_VEC_VAR_TYPE(float3)
ADD_VEC_VAR_TYPE(float4)
ADD_VEC_VAR_TYPE(uint2)
ADD_VEC_VAR_TYPE(int2)

#undef ADD_VEC_VAR_TYPE

template<typename Vec, typename Type>
bool Gui::Widget::slider(std::string_view label, Vec& var, Type minValue, Type maxValue, bool sameLine)
{
	if constexpr (is_vector_v<Vec> && is_vector_type<Type, Vec>)
	{
		return _gui ? _gui->_wrapper->addVecSlider(label, var, minValue, maxValue, sameLine) : false;
	}
	else
	{
		return _gui ? _gui->_wrapper->addScalarSlider(label, var, minValue, maxValue, sameLine) : false;
	}
}

#define ADD_SCALAR_SLIDER_TYPE(TypeName) template bool Gui::Widget::slider<TypeName>(std::string_view label, TypeName& var, TypeName minValue, TypeName maxValue, bool sameLine);

ADD_SCALAR_SLIDER_TYPE(int32_t)
ADD_SCALAR_SLIDER_TYPE(uint32_t)
ADD_SCALAR_SLIDER_TYPE(uint64_t)
ADD_SCALAR_SLIDER_TYPE(float_t)
ADD_SCALAR_SLIDER_TYPE(double_t)

#undef ADD_SCALAR_SLIDER_TYPE

#define ADD_VEC_SLIDER_TYPE(TypeName) template bool Gui::Widget::slider<TypeName>(std::string_view label, TypeName& var, typename TypeName::value_type minValue, typename TypeName::value_type maxValue, bool sameLine);

ADD_VEC_SLIDER_TYPE(int2)

#undef ADD_VEC_SLIDER_TYPE

void Gui::Widget::text(std::string_view text, bool sameLine)
{
	if (_gui)
	{
		_gui->_wrapper->addText(text, sameLine);
	}
}

void Gui::Widget::textWrapped(std::string_view text)
{
	if (_gui)
	{
		_gui->_wrapper->addTextWrapped(text);
	}
}

bool Gui::Widget::textbox(std::string_view label, std::string& text, TextFlags flags)
{
	return _gui ? _gui->_wrapper->addTextbox(label, text, 1, flags) : false;
}

bool Gui::Widget::textboxMultiline(std::string_view label, const std::vector<std::string>& text, std::vector<std::string>& textEntries)
{
	return _gui ? _gui->_wrapper->addMultiTextbox(label, text, textEntries) : false;
}

void Gui::Widget::tooltip(std::string_view text, bool sameLine)
{
	if (_gui)
	{
		_gui->_wrapper->addTooltip(text, sameLine);
	}
}

bool Gui::Widget::rgbColour(std::string_view label, float3& var, bool sameLine)
{
	return _gui ? _gui->_wrapper->addRgbColor(label, var, sameLine) : false;
}


bool Gui::Widget::rgbaColour(std::string_view label, float4& var, bool sameLine)
{
	return _gui ? _gui->_wrapper->addRgbaColor(label, var, sameLine) : false;
}

void Gui::Widget::image(std::string_view label, const egkr::Texture2D& image, vk::Sampler sampler, float2 size, bool maintainRatio, bool sameLine)
{
	if (_gui)
	{
		_gui->_wrapper->addImage(label, image, sampler, size, maintainRatio, sameLine);
	}
}

void Gui::Widget::imageButton(std::string_view label, const egkr::Texture2D& image, vk::Sampler sampler, float2 size, bool maintainRatio, bool sameLine )
{
	if (_gui)
	{
		_gui->_wrapper->addImageButton(label, image, sampler, size, maintainRatio, sameLine);
	}
}

template<typename MatrixType>
bool Gui::Widget::matrix(std::string_view label, MatrixType& var, float minValue, float maxValue, bool sameLine)
{
	return _gui ? _gui->_wrapper->addMatrixVar(label, var, minValue, maxValue, sameLine) : false;
}

Gui::Group::Group(Gui* gui, std::string_view label, bool beginExpanded)
{
	if (gui && gui->_wrapper->beginGroup(label, beginExpanded))
	{
		_gui = gui;
	}
}

bool Gui::Group::isOpen() const
{
	return _gui != nullptr;
}

Gui::Group::~Group()
{
	release();
}

void Gui::Group::release()
{
	if (_gui)
	{
		_gui->_wrapper->endGroup();
		_gui = nullptr;
	}
}

Gui::Window::Window(Gui* gui, std::string_view label, uint2 size, uint2 position, Gui::WindowFlags flags)
{
	auto open = true;
	if (gui->_wrapper->pushWindow(label, open, size, position, flags))
	{
		_gui = gui;
	}
}

Gui::Window::Window(Gui* gui, std::string_view label, bool& open, uint2 size, uint2 position, Gui::WindowFlags flags)
{
	if (gui->_wrapper->pushWindow(label, open, size, position, flags))
	{
		_gui = gui;
	}
}

Gui::Window::~Window()
{
	release();
}

void Gui::Window::release()
{
	if (_gui)
	{
		_gui->_wrapper->popWindow();
		_gui = nullptr;
	}
}

void Gui::Window::columns(uint32_t numColumns)
{
	if (_gui)
	{
		_gui->_wrapper->beginColumns(numColumns);
	}
}

void Gui::Window::nextColumn()
{
	if (_gui)
	{
		_gui->_wrapper->nextColumn();
	}
}

void Gui::Window::windowSize(uint32_t width, uint32_t height)
{
	if (_gui)
	{
		_gui->_wrapper->setCurrentWindowSize(width, height);
	}
}

void Gui::Window::windowPosition(uint32_t x, uint32_t y)
{
	if (_gui)
	{
		_gui->_wrapper->setCurrentWindowPosition(x, y);
	}
}

