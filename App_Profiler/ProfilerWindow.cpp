#include "ProfilerWindow.h"

#include <fancy_imgui/imgui.h>
#include <fancy_core/Profiling.h>
#include <fancy_core/FancyCoreDefines.h>
#include <fancy_core/Log.h>
#include <fancy_core/MathIncludes.h>
#include "fancy_imgui/imgui_internal.h"
#include <list>


using namespace Fancy;

ProfilerWindow::ProfilerWindow()
{
}


ProfilerWindow::~ProfilerWindow()
{
}

char TextBuf[2048];
const float kZoneElementHeight = 20.0f;
const float kZoneElementHeight_WithPadding = 25.0f;
const float kRulerMarkerVerticalSize = 40.0f;
const float kSubRulerMarkerVerticalSize = 15.0f;

const char* FormatString(const char* aFmt, ...)
{
  va_list args;
  va_start(args, aFmt);

  const int neededSize = vsnprintf(nullptr, 0u, aFmt, args) + 1;
  ASSERT(neededSize < ARRAY_LENGTH(TextBuf));
  const int offset = vsnprintf(TextBuf, ARRAY_LENGTH(TextBuf), aFmt, args);
  TextBuf[offset + 1] = '\0';
  va_end(args);
  return TextBuf;
}

bool ColorButton(const char* aLabel, const ImVec2& aPos, const ImVec2& aSize, const ImVec4& aColor)
{
  ImGui::SetCursorPos(aPos);
  ImGui::PushStyleColor(ImGuiCol_Button, aColor);
  const bool pressed = ImGui::Button(aLabel, aSize);
  ImGui::PopStyleColor(1);
  return pressed;
}

ImVec4 GetColorForTag(uint8 aTag) 
{
  switch(aTag)
  {
  case 0: return ImVec4(0.7f, 0.0f, 0.0f, 0.5f);
  default: return ImVec4(0.5f, 0.5f, 0.5f, 0.5f);
  }
}

struct ScaleArgs
{
  float myMsToPixelScale = 1.0f;
  float myScale = 1.0f;
};

enum TimeUnit
{
  Milliseconds,
  Microseconds,
  Nanoseconds,

  Last = Nanoseconds
};

const char* TimeUnitToString(TimeUnit aUnit)
{
  switch(aUnit) 
  { 
    case Milliseconds: return "ms";
    case Microseconds: return "us";
    case Nanoseconds: return "ns";
    default: ASSERT(false); return "";
  }
}

struct NodeRenderArgs
{
  float64 myFrameStart;
  ImVec2 myStartPos;
};

void RenderNodeRecursive(const Profiling::SampleNode& aNode, const ScaleArgs& someScaleArgs, const NodeRenderArgs& someRenderArgs, int aDepth)
{
  ImVec2 pos, size;
  pos.x = someRenderArgs.myStartPos.x + (aNode.myStart - someRenderArgs.myFrameStart) * someScaleArgs.myMsToPixelScale;
  pos.y = someRenderArgs.myStartPos.y + kZoneElementHeight_WithPadding * aDepth;
  size.x = aNode.myDuration * someScaleArgs.myMsToPixelScale;
  size.y = kZoneElementHeight;

  const Profiling::SampleNodeInfo& nodeInfo = Profiling::GetSampleInfo(aNode.myNodeInfo);

  ColorButton(FormatString("%s: %.3f", nodeInfo.myName, (float)aNode.myDuration), pos, size, GetColorForTag(nodeInfo.myTag));

  if (aNode.myChild != UINT_MAX)
  {
    const Profiling::SampleNode& firstChild = Profiling::GetSample(aNode.myChild);
    RenderNodeRecursive(firstChild, someScaleArgs, someRenderArgs, aDepth + 1);
  }

  if (aNode.myNext != UINT_MAX)
  {
    const Profiling::SampleNode& nextNode = Profiling::GetSample(aNode.myNext);
    RenderNodeRecursive(nextNode, someScaleArgs, someRenderArgs, aDepth);
  }
}

void RenderRuler(const ScaleArgs& someScaleArgs)
{
  ImGuiWindow* window = ImGui::GetCurrentWindow();

  TimeUnit mainMarkerUnit = TimeUnit::Milliseconds;
  float mainMarkerOffset = someScaleArgs.myMsToPixelScale;
  float mainMarkerDuration = 1;  // Duration in mainMarkerUnit
  while (mainMarkerOffset > 500 && mainMarkerUnit < TimeUnit::Last)
  {
    mainMarkerUnit = (TimeUnit) ((uint) mainMarkerUnit + 1);
    const float divide = 10.0f;
    mainMarkerOffset *= (1.0f / divide);
    mainMarkerDuration *= (1000.0f / divide);
  }

  const float subMarkerOffset = mainMarkerOffset / 10.0f;
  
  const float subMarkerVerticalOffset = (kRulerMarkerVerticalSize - kSubRulerMarkerVerticalSize) * 0.5f;
  const ImU32 markerColor = ImGui::ColorConvertFloat4ToU32(ImVec4(.5f, .5f, .5f, .8f));
  const float mainMarkerThickness = 1.5f;
  const float subMarkerThickness = 1.0f;

  const uint numMainMarkers = (uint)(ImGui::GetWindowSize().x / mainMarkerOffset);
  const uint numSubMarkers = 9;
  ImVec2 startPos = ImGui::GetCursorPos();
  ImVec2 pos = startPos;
  pos.x += ImGui::GetWindowPos().x;
  pos.y += ImGui::GetWindowPos().y;
  int currMainMarkerTime = 0;
  for (uint iMain = 0u; iMain < numMainMarkers; ++iMain)
  {
    ImVec2 start = pos, end = pos;

    end.y += kRulerMarkerVerticalSize;
    window->DrawList->AddLine(start, end, markerColor, mainMarkerThickness);

    ImVec2 labelPos = end;
    labelPos.y += 5;
    window->DrawList->AddText(labelPos, markerColor, FormatString("%d%s", currMainMarkerTime, TimeUnitToString(mainMarkerUnit)));

    for (uint iSub = 0u; iSub < numSubMarkers; ++iSub)
    {
      start.x += subMarkerOffset;
      end.x += subMarkerOffset;
      start.y = pos.y + subMarkerVerticalOffset;
      end.y = pos.y + (kRulerMarkerVerticalSize - subMarkerVerticalOffset);

      window->DrawList->AddLine(start, end, markerColor, subMarkerThickness);
    }
  
    pos.x += mainMarkerOffset;
    currMainMarkerTime += mainMarkerDuration;
  }

  ImGui::SetCursorPosY(startPos.y + kRulerMarkerVerticalSize + 20);
  ImGui::SetCursorPosX(startPos.x);
}

void ProfilerWindow::Render()
{
  // TODO: Clipping of elements not on the screen
  // TODO: Determine horizontal time-bounds of the profiler-window and figure out which frame-datas to render

  const float baseHorizontalScale = 1280.0f / 16.0f;  // 16ms scale to 1280px
  ScaleArgs scaleArgs;
  scaleArgs.myMsToPixelScale = baseHorizontalScale * myScale;
  scaleArgs.myScale = myScale;

  const Profiling::FrameData& lastFrame = Profiling::GetLastFrame();
  const float64 maxTime = lastFrame.myStart;
  const float64 minTime = maxTime - ImGui::GetWindowWidth() / scaleArgs.myMsToPixelScale;

  ImGui::Begin("Profiler");

  // RenderRuler(scaleArgs);

  // Step back in time to look for the first frame to render in the profiler window
  const Profiling::FrameData* framePtr = &lastFrame;
  while (framePtr->myPrev != UINT_MAX && framePtr->myStart > minTime)
    framePtr = &Profiling::GetFrame(framePtr->myPrev);

  ImVec2 frameStartPos = ImGui::GetCursorPos();
  while(framePtr->myNext != UINT_MAX && framePtr->myStart <= maxTime)
  {
    NodeRenderArgs nodeRenderArgs;
    nodeRenderArgs.myFrameStart = framePtr->myStart;
    nodeRenderArgs.myStartPos = frameStartPos;

    const Profiling::SampleNode& node = Profiling::GetSample(framePtr->myFirstSample);
    RenderNodeRecursive(node, scaleArgs, nodeRenderArgs, 0);

    frameStartPos.x += framePtr->myDuration * scaleArgs.myMsToPixelScale;
    framePtr = &Profiling::GetFrame(framePtr->myNext);
  }
    
  ImGui::End();
}
