---
name: review-ui
description: UI/UX review of FreeCAD screenshots -- analyze layouts, consistency, accessibility, and suggest optimizations
argument-hint: "<screenshot_path>"
allowed-tools: Read, Grep, Glob, Bash
user-invocable: true
agent: ui-ux-reviewer
---

# UI/UX Review

Analyze a FreeCAD UI screenshot and provide detailed design review with optimization suggestions.

## Arguments

`$0` -- Path to screenshot file to analyze

## Steps

1. **Read the screenshot** at `$0` using the Read tool (supports images)

2. **Analyze** across these dimensions:

### Layout & Spacing
- Widget alignment, margins, padding consistency
- Proper use of group boxes and form layouts
- Spacer/stretch usage

### Visual Hierarchy
- Primary/secondary element distinction
- Heading sizes and logical grouping
- Section separation

### Labeling
- Clear, concise labels with consistent terminology
- Sentence case for labels, title case for headings
- Tooltip indicators for non-obvious controls

### Interaction Design
- Logical control flow (top-to-bottom)
- Appropriate widget types for data types
- Default values, validation indicators

### Accessibility
- Color contrast (WCAG AA)
- Font size readability (min 11px)
- Keyboard navigability
- No color-only differentiators

### Consistency
- Matches FreeCAD's existing UI patterns
- Icon style consistency
- Theme compatibility (light + dark mode)

### CAD Industry Comparison
- Compare with SolidWorks, Fusion 360, Blender patterns
- Property panel patterns, toolbar density

3. **Produce report** with scored assessment, categorized issues, and specific actionable suggestions.

## FreeCAD UI Layout Reference

```
+------------------------------------------+
| Menu Bar                                 |
| Toolbar(s)                               |
+--------+------------------------+--------+
| Model  |                        |Property|
| Tree   |    3D Viewport         | Panel  |
+--------+------------------------+--------+
|        Task Panel / Report View          |
+------------------------------------------+
```
