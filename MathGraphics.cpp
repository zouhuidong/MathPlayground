#include <gl/freeglut.h>
#include "MathGraphics.h"
#include "glutUtils.h"
#include <cmath>
#include <iostream>

#define M_PI 3.14159265358979323846f

// 繪製座標軸刻度和標註（通用函數）
// axis: 0=X軸, 1=Y軸, 2=Z軸
void DrawAxisScale(int axis, float minVal, float maxVal, float tickStep) {
	float color[3][3] = {
		{1.0f, 0.0f, 0.0f}, // X軸 - 紅色
		{0.0f, 1.0f, 0.0f}, // Y軸 - 綠色
		{0.0f, 0.0f, 1.0f}  // Z軸 - 藍色
	};

	char axisLabel[] = { 'X', 'Y', 'Z' };

	glLineWidth(1.0f);
	glColor3f(color[axis][0], color[axis][1], color[axis][2]);

	// 繪製刻度和標註
	for (float val = minVal; val <= maxVal; val += tickStep) {
		bool isMajorTick = fabsf(val - roundf(val)) < 0.001f;
		float scaleLength = isMajorTick ? 0.2f : 0.1f;

		// 根據軸選擇座標
		float pos[3] = { 0.0f, 0.0f, 0.0f };
		float tickOffset[3] = { 0.0f, 0.0f, 0.0f };
		float textOffset[3] = { 0.0f, 0.0f, 0.0f };

		switch (axis) {
		case 0: // X軸
			pos[0] = val;
			tickOffset[1] = -scaleLength;
			textOffset[0] = -0.1f;
			textOffset[1] = -0.2f;
			break;
		case 1: // Y軸
			pos[1] = val;
			tickOffset[0] = -scaleLength;
			textOffset[0] = -0.2f;
			textOffset[1] = -0.1f;
			break;
		case 2: // Z軸
			pos[2] = val;
			tickOffset[0] = -scaleLength;
			textOffset[0] = -0.2f;
			textOffset[2] = -0.1f;
			break;
		}

		// 繪製刻度線
		glBegin(GL_LINES);
		glVertex3f(pos[0], pos[1], pos[2]);
		glVertex3f(pos[0] + tickOffset[0], pos[1] + tickOffset[1], pos[2] + tickOffset[2]);
		glEnd();

		// 在整數位置添加數字標註
		if (isMajorTick) {
			char label[10];
			sprintf_s(label, "%.0f", val);
			glutUtils::renderBitmapString(
				pos[0] + textOffset[0],
				pos[1] + textOffset[1],
				pos[2] + textOffset[2],
				label
			);
		}
	}

	// 軸標籤
	float labelPos[3] = { 0.0f, 0.0f, 0.0f };
	labelPos[axis] = maxVal + 0.2f;

	char axisName[2] = { axisLabel[axis], '\0' };
	glutUtils::renderBitmapString(labelPos[0], labelPos[1], labelPos[2], axisName);
}

// 繪製座標系軸線
void DrawAxisLines()
{
	// 繪製座標系
	glLineWidth(2.0f);

	glColor3f(1.0f, 0.0f, 0.0f); // X軸 - 紅色
	glBegin(GL_LINES);
	glVertex3f(-5.0f, 0.0f, 0.0f);
	glVertex3f(5.0f, 0.0f, 0.0f);
	glEnd();

	glColor3f(0.0f, 1.0f, 0.0f); // Y軸 - 綠色
	glBegin(GL_LINES);
	glVertex3f(0.0f, -5.0f, 0.0f);
	glVertex3f(0.0f, 5.0f, 0.0f);
	glEnd();

	glColor3f(0.0f, 0.0f, 1.0f); // Z軸 - 藍色
	glBegin(GL_LINES);
	glVertex3f(0.0f, 0.0f, -5.0f);
	glVertex3f(0.0f, 0.0f, 5.0f);
	glEnd();

	glLineWidth(1.0f);
}

void DrawAxes()
{
	// 繪製座標系軸線
	DrawAxisLines();

	// 繪製座標軸標尺和標註
	DrawAxisScale(0, -5.0f, 5.0f, 0.1f);
	DrawAxisScale(1, -5.0f, 5.0f, 0.1f);
	DrawAxisScale(2, -5.0f, 5.0f, 0.1f);
}

void DrawVector(const af::array& begin, const af::array& end, glColor color, bool drawCubeWireframe)
{
    if (begin.dims(0) != 3 || end.dims(0) != 3)
    {
        std::cerr << "Error: begin and end arrays must be 3D vectors." << std::endl;
        return;
    }
    float* begin_ptr = begin.as(f32).host<float>();
    float* end_ptr = end.as(f32).host<float>();
    float vec_diff[3] = { 0 };
    float square_length = 0;

    for (int i = 0; i < 3; i++)
    {
        vec_diff[i] = end_ptr[i] - begin_ptr[i];
        square_length += vec_diff[i] * vec_diff[i];
    }

    // 設置顏色
    glColor3f(color.r, color.g, color.b);

    // 繪製箭頭主體
    glBegin(GL_LINES);
    glVertex3f(begin_ptr[0], begin_ptr[1], begin_ptr[2]);
    glVertex3f(end_ptr[0], end_ptr[1], end_ptr[2]);
    glEnd();

    // 繪製3D箭頭頭部
	float arrowSize = 0.1f; // 箭頭大小比例
    float arrowHeadLength = arrowSize * sqrt(square_length);
    float arrowHeadAngle = 30.0f * M_PI / 180.0f; // 30度角

    // 計算箭頭方向的單位向量
    float vec_length = sqrt(vec_diff[0] * vec_diff[0] + vec_diff[1] * vec_diff[1] + vec_diff[2] * vec_diff[2]);
    float dir_x = vec_diff[0] / vec_length;
    float dir_y = vec_diff[1] / vec_length;
    float dir_z = vec_diff[2] / vec_length;

    // 找到一個與箭頭方向垂直的向量（用於確定箭頭側翼的方向）
    float perp_x, perp_y, perp_z;
    if (fabs(dir_x) < 0.9) {
        // 如果箭頭方向不接近X軸，則使用X軸作爲參考
        perp_x = 1.0f;
        perp_y = 0.0f;
        perp_z = 0.0f;
    }
    else {
        // 否則使用Y軸作爲參考
        perp_x = 0.0f;
        perp_y = 1.0f;
        perp_z = 0.0f;
    }

    // 計算垂直於箭頭方向的向量（叉乘）
    float cross_x = dir_y * perp_z - dir_z * perp_y;
    float cross_y = dir_z * perp_x - dir_x * perp_z;
    float cross_z = dir_x * perp_y - dir_y * perp_x;

    // 歸一化叉乘結果
    float cross_length = sqrt(cross_x * cross_x + cross_y * cross_y + cross_z * cross_z);
    cross_x /= cross_length;
    cross_y /= cross_length;
    cross_z /= cross_length;

    // 再計算另一個垂直向量（與箭頭方向和第一個垂直向量都垂直）
    float cross2_x = dir_y * cross_z - dir_z * cross_y;
    float cross2_y = dir_z * cross_x - dir_x * cross_z;
    float cross2_z = dir_x * cross_y - dir_y * cross_x;

    // 歸一化第二個叉乘結果
    float cross2_length = sqrt(cross2_x * cross2_x + cross2_y * cross2_y + cross2_z * cross2_z);
    cross2_x /= cross2_length;
    cross2_y /= cross2_length;
    cross2_z /= cross2_length;

    // 箭頭底部中心點位置（箭頭杆的末端）
    float base_x = end_ptr[0] - arrowHeadLength * dir_x;
    float base_y = end_ptr[1] - arrowHeadLength * dir_y;
    float base_z = end_ptr[2] - arrowHeadLength * dir_z;

    // 計算箭頭底部圓的半徑
    float arrowHeadRadius = arrowHeadLength * tan(arrowHeadAngle);

    // 繪製箭頭頭部（錐體）
    int segments = 16; // 分割數，決定錐體的平滑度
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(end_ptr[0], end_ptr[1], end_ptr[2]); // 錐體頂點

    // 生成底部圓形的頂點
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * M_PI * float(i) / float(segments);
        float x = base_x + arrowHeadRadius * (cos(angle) * cross_x + sin(angle) * cross2_x);
        float y = base_y + arrowHeadRadius * (cos(angle) * cross_y + sin(angle) * cross2_y);
        float z = base_z + arrowHeadRadius * (cos(angle) * cross_z + sin(angle) * cross2_z);
        glVertex3f(x, y, z);
    }
    glEnd();

    // 繪製立方體虛線框邏輯，如果 drawCubeWireframe 爲 true 則執行
    if (drawCubeWireframe) {
        // 開啓虛線模式
        glEnable(GL_LINE_STIPPLE);
        // 設置虛線圖案，這裏是 4 個像素實線、4 個像素空白的重複模式，可根據需要調整
        glLineStipple(1, 0x00FF);

        // 確定立方體的八個頂點，根據 begin 和 end 在各軸的極值確定
        float min_x = std::min(begin_ptr[0], end_ptr[0]);
        float max_x = std::max(begin_ptr[0], end_ptr[0]);
        float min_y = std::min(begin_ptr[1], end_ptr[1]);
        float max_y = std::max(begin_ptr[1], end_ptr[1]);
        float min_z = std::min(begin_ptr[2], end_ptr[2]);
        float max_z = std::max(begin_ptr[2], end_ptr[2]);

        // 定義立方體的 8 個頂點
        float cubeVertices[8][3] = {
            {min_x, min_y, min_z}, {max_x, min_y, min_z},
            {max_x, max_y, min_z}, {min_x, max_y, min_z},
            {min_x, min_y, max_z}, {max_x, min_y, max_z},
            {max_x, max_y, max_z}, {min_x, max_y, max_z}
        };

        // 定義立方體的 12 條棱邊，每條邊由兩個頂點索引組成
        int cubeEdges[12][2] = {
            {0, 1}, {1, 2}, {2, 3}, {3, 0},  // 底面
            {4, 5}, {5, 6}, {6, 7}, {7, 4},  // 頂面
            {0, 4}, {1, 5}, {2, 6}, {3, 7}   // 垂直邊
        };

        // 繪製立方體的棱邊
        glBegin(GL_LINES);
        for (int i = 0; i < 12; i++) {
            int v1 = cubeEdges[i][0];
            int v2 = cubeEdges[i][1];
            glVertex3f(cubeVertices[v1][0], cubeVertices[v1][1], cubeVertices[v1][2]);
            glVertex3f(cubeVertices[v2][0], cubeVertices[v2][1], cubeVertices[v2][2]);
        }
        glEnd();

        // 關閉虛線模式
        glDisable(GL_LINE_STIPPLE);
    }

    af::freeHost(begin_ptr);
    af::freeHost(end_ptr);
}