/////////////////////////////////////////////////////
//
//	glutUtils.h
//	OpenGL Utility Functions for GLUT
//
//	Features:
//	- Rendering Text
// 
//	Author: huidong <mailhuid@163.com>
//	Date:	2025-06-26
//
//

#pragma once

namespace glutUtils
{
	// Set the stroke font for rendering text
	void setStrokeFont(void* font);

	// Set the bitmap font for rendering text
	void setBitmapFont(void* font);

	// Set the stroke font size for rendering text
	// 這裏的字號代表字體高度
	void setStrokeFontSize(float size);

	// Render a string using stroke font at specified position and rotation
	// 
	// 通過此方法輸出的文本看起來像是三維場景的一部分。
	void renderStrokeFontString(
		float x,
		float y,
		float z,
		const char* string,
		float dgree_x = 0,
		float dgree_y = 0,
		float dgree_z = 0);

	// Render a string using bitmap font at specified position
	// 
	// 不支持縮放、旋轉等變換，因爲 Bitmap 字體是 2D 渲染的。
	// 在正交投影（setOrthographicProjection）下使用此函數可以將文本精確渲染到屏幕的某個位置（此時 z 軸被忽略）。
	// 在透視投影（restorePerspectiveProjection）下使用此函數可以將文本輸出到三維空間的指定位置，且始終保持平面朝向觀察者。
	void renderBitmapString(
		float x,
		float y,
		float z,
		const char* string);

	// Set orthographic projection for 2D rendering
	void setOrthographicProjection();

	// Set perspective projection for 3D rendering
	void restorePerspectiveProjection();
}
