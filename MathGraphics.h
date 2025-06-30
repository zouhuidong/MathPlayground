/////////////////////////////////////////////////
//
//	MathGraphics.h
// 
//	數學相關圖形繪製函數庫
//
//	Author:	huidong <mailhuid@163.com>
//	Date:	2025-06-28
//

#pragma once

#include <arrayfire.h>  // 包含 ArrayFire 庫的頭文件

struct glColor {
	GLfloat r, g, b;
};

void DrawAxes();

// 繪製向量，必須爲 3*1 向量
void DrawVector(const af::array& begin, const af::array& end, glColor color, bool drawCubeWireframe = false);
