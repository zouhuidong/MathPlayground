#include <gl/freeglut.h>
#include <gl/glui.h>
#include <iostream>
#include <cmath>
#include <string>
#include "glutUtils.h"
#include "MathGraphics.h"


///////////////////////////////////
//	程序框架全局變量
///////////////////////////////////

// 主窗口 id
int main_window;

int windowWidth = 800;
int windowHeight = 600;

float cameraRadianX = 0.0f;
float cameraRadianY = 0.0f;
float cameraDistance = 5.0f; // 相機距離原點的球座標半徑

int lastMouseX = 0;
int lastMouseY = 0;
bool mouseDragging = false;

///////////////////////////////////
//	用戶全局變量
///////////////////////////////////

// 受控向量
af::array vec1 = af::constant(1, 3, 1)/*{1.f, 1.f, 1.f}*/;

// 三維矩陣信息
float arrMat[3][3][3] = {
{
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f
},
{
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f
},
{
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f
}
};

// 沿三個軸的旋轉
float radianRotate[3] = { 0 };

enum RotationOrder
{
	ROTATE_XYZ = 0, // X -> Y -> Z
	ROTATE_XZY,     // X -> Z -> Y
	ROTATE_YXZ,     // Y -> X -> Z
	ROTATE_YZX,     // Y -> Z -> X
	ROTATE_ZXY,     // Z -> X -> Y
	ROTATE_ZYX      // Z -> Y -> X
};

enum RotationDirection
{
	ROTATE_INTERNAL = 0, // 內旋
	ROTATE_EXTERNAL,     // 外旋
};

int rotationOrder = ROTATE_XYZ;				// 旋轉順序
int rotationDirection = ROTATE_EXTERNAL;	// 旋轉方向

// 指定是否降維到二維
//bool isTwoDimensional = false;

// 旋轉變換完成以後得到的基向量
af::array matBase;

// 旋轉後的 vec1
af::array vec1_r;

// 是否顯示立方體虛線框
int doesShowWireframe = true;

// 運動軌跡
struct Point3f
{
	float x, y, z;
};
std::vector<Point3f> vecPath;
int doesShowPath = true;

///////////////////////////////////
//	用戶繪製、計算相關函數
///////////////////////////////////

void RecalcMatMul()
{
	af::array matX(3, 3, (float*)arrMat[0]);
	af::array matY(3, 3, (float*)arrMat[1]);
	af::array matZ(3, 3, (float*)arrMat[2]);

	LPCSTR strErrInvalidOrder = "Error: Invalid rotation order.";
	LPCSTR strErrInvalidDirection = "Error: Invalid rotation direction.";

	// 根據旋轉順序計算最終的旋轉矩陣
	if (rotationDirection == ROTATE_EXTERNAL)
	{
		switch (rotationOrder)
		{
		case ROTATE_XYZ:
			matBase = af::matmul(matZ, matY, matX);
			break;
		case ROTATE_XZY:
			matBase = af::matmul(matY, matZ, matX);
			break;
		case ROTATE_YXZ:
			matBase = af::matmul(matZ, matX, matY);
			break;
		case ROTATE_YZX:
			matBase = af::matmul(matX, matZ, matY);
			break;
		case ROTATE_ZXY:
			matBase = af::matmul(matY, matX, matZ);
			break;
		case ROTATE_ZYX:
			matBase = af::matmul(matX, matY, matZ);
			break;
		default:
			std::cerr << strErrInvalidOrder << std::endl;
			return;
		}
	}
	else if (rotationDirection == ROTATE_INTERNAL)
	{
		switch (rotationOrder)
		{
		case ROTATE_XYZ:
			matBase = af::matmul(matX, matY, matZ);
			break;
		case ROTATE_XZY:
			matBase = af::matmul(matX, matZ, matY);
			break;
		case ROTATE_YXZ:
			matBase = af::matmul(matY, matX, matZ);
			break;
		case ROTATE_YZX:
			matBase = af::matmul(matY, matZ, matX);
			break;
		case ROTATE_ZXY:
			matBase = af::matmul(matZ, matX, matY);
			break;
		case ROTATE_ZYX:
			matBase = af::matmul(matZ, matY, matX);
			break;
		default:
			std::cerr << strErrInvalidOrder << std::endl;
			return;
		}
	}
	else
	{
		std::cerr << strErrInvalidDirection << std::endl;
	}

	vec1_r = af::matmul(matBase, vec1);

#ifdef _DEBUG
	af_print(matBase);
	af_print(vec1_r);
#endif

}

// 根據旋轉角度重新計算
void RecalcRotation()
{
	// 預計算三角函數值
	float cx = cosf(radianRotate[0]);
	float sx = sinf(radianRotate[0]);
	float cy = cosf(radianRotate[1]);
	float sy = sinf(radianRotate[1]);
	float cz = cosf(radianRotate[2]);
	float sz = sinf(radianRotate[2]);

	// 分別計算三個軸的旋轉矩陣（3x3）
	float r_mat[3][3][3] = {
		{
			1.0f, 0.0f, 0.0f,
			0.0f, cx, -sx,
			0.0f, sx, cx
		},
		{
			cy, 0.0f, sy,
			0.0f, 1.0f, 0.0f,
			-sy, 0.0f, cy
		},
		{
			cz, -sz, 0.0f,
			sz, cz, 0.0f,
			0.0f, 0.0f, 1.0f
		}
	};

	for (int i = 0; i < 3 * 3 * 3; i++)
	{
		((float*)arrMat)[i] = ((float*)r_mat)[i];
	}
}

void RecalcAll()
{
	RecalcRotation();
	RecalcMatMul();
}

// 用戶繪製函數
void MyDisplay()
{
	// 原點
	af::array vecOrg = { 0, 0, 0 };

	// 基向量
	DrawVector(vecOrg, matBase.col(0), { 0.8f, 0.0f, 0.0f });
	DrawVector(vecOrg, matBase.col(1), { 0.0f, 0.8f, 0.0f });
	DrawVector(vecOrg, matBase.col(2), { 0.0f, 0.0f, 0.8f });


	// Temp
	/*af::array matX(3, 3, (float*)arrMat[0]);
	af::array matY(3, 3, (float*)arrMat[1]);
	af::array matZ(3, 3, (float*)arrMat[2]);
	
	af::array x_zR = af::matmul(matZ, { 1.0f, 0.0f, 0.0f });
	float* ptr1 = x_zR.host<float>();
	DrawVector(vecOrg, x_zR, { 0.0f, 0.7f, 0.0f }, false);
	glutUtils::renderBitmapString(ptr1[0], ptr1[1], ptr1[2], "y(x_rotated)");
	af::freeHost(ptr1);

	af::array z_xyR = af::matmul(matY, matX, { 0.0f, 0.0f, 1.0f });
	float* ptr2 = z_xyR.host<float>();
	DrawVector(vecOrg, z_xyR, { 0.0f, 0.0f, 0.7f }, false);
	glutUtils::renderBitmapString(z_ptr[0], z_ptr[1], z_ptr[2], "z_(xy_rotated)");
	af::freeHost(z_ptr);*/
	/*af::array matX(3, 3, (float*)arrMat[0]);
	af::array matY(3, 3, (float*)arrMat[1]);
	af::array matZ(3, 3, (float*)arrMat[2]);

	af::array ZYx = af::matmul(matZ, matY, { 1.0f, 0.0f, 0.0f });
	float* ZYx_ptr = ZYx.host<float>();
	DrawVector(vecOrg, ZYx, { 0.7f, 0.0f, 0.0f }, false);
	glutUtils::renderBitmapString(ZYx_ptr[0], ZYx_ptr[1], ZYx_ptr[2], "ZY(1, 0, 0)'");
	af::freeHost(ZYx_ptr);

	af::array Zy = af::matmul(matZ, { 0.0f, 1.0f, 0.0f });
	float* Zy_ptr = Zy.host<float>();
	DrawVector(vecOrg, Zy, { 0.0f, 0.7f, 0.0f }, false);
	glutUtils::renderBitmapString(Zy_ptr[0], Zy_ptr[1], Zy_ptr[2], "Z(0, 1, 0)'");
	af::freeHost(Zy_ptr);

	DrawVector(vecOrg, {0, 0, 1}, { 0.0f, 0.0f, 0.7f }, false);
	glutUtils::renderBitmapString(0, 0, 1, "(0, 0, 1)'");*/


	// vec1 的旋轉結果
	DrawVector(vecOrg, vec1_r, { 1.0f, 1.0f, 1.0f }, doesShowWireframe);

	// 運動軌跡
	if (doesShowPath)
	{
		// 開始繪製點
		glBegin(GL_POINTS);
		glColor3f(1.0f, 0.0f, 0.0f);  // 設置點的顏色爲紅色

		// 遍歷所有點並繪製
		for (const auto& point : vecPath)
		{
			glVertex3f(point.x, point.y, point.z);
		}

		glEnd();
	}
}

///////////////////////////////////
//	程序框架回調函數
///////////////////////////////////

// 顯示回調函數
void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	// 設置相機位置和方向
	gluLookAt(
		cameraDistance * sinf(cameraRadianY) * cosf(cameraRadianX),
		cameraDistance * sinf(cameraRadianY) * sinf(cameraRadianX),
		cameraDistance * cosf(cameraRadianY),
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f
	);

	DrawAxes();
	MyDisplay();

	glutSwapBuffers();
}

// 鼠標回調函數
void mouse(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON) {
		if (state == GLUT_DOWN) {
			mouseDragging = true;
			lastMouseX = x;
			lastMouseY = y;
		}
		else {
			mouseDragging = false;
		}
	}
}

// 鼠標移動回調函數
void motion(int x, int y) {
	if (mouseDragging) {
		int dx = x - lastMouseX;
		int dy = y - lastMouseY;

		cameraRadianX -= dx * 0.01f;
		cameraRadianY -= dy * 0.01f;

		// 限制仰角範圍
		if (cameraRadianY < 0.1f) cameraRadianY = 0.1f;
		if (cameraRadianY > 3.14f - 0.1f) cameraRadianY = 3.14f - 0.1f;

		lastMouseX = x;
		lastMouseY = y;

		GLUI_Master.sync_live_all();
		glutPostRedisplay();
	}
}

// 鼠标滚轮回调函数
void mouseWheel(int wheel, int direction, int x, int y) {
	// 缩放逻辑：滚轮方向控制距离增减
	if (direction > 0) {
		cameraDistance = std::max(1.0f, cameraDistance - 1.0f); // 最小距离1
	}
	else {
		cameraDistance = std::min(20.0f, cameraDistance + 1.0f); // 最大距离20（可选限制）
	}
	glutPostRedisplay();
}

// 鍵盤迴調函數
void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 27: // ESC鍵退出
		exit(0);
		break;
	}
}

// 窗口大小改變回調函數
void reshape(int width, int height) {
	if (height == 0) height = 1;

	windowWidth = width;
	windowHeight = height;

	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);

	glMatrixMode(GL_MODELVIEW);
}

void idle()
{
	glutSetWindow(main_window);
	glutPostRedisplay();
}

///////////////////////////////////
//	用戶控件設置相關函數
///////////////////////////////////

// 爲一個矩陣創建值控制面板
void CreateMatrixValPanel(GLUI* glui, const char* name, float mat[3][3])
{
	auto panel = glui->add_rollout(name);
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			auto spinner = glui->add_spinner_to_panel(panel, "", GLUI_SPINNER_FLOAT, &mat[i][j], -1, (GLUI_CB)[](int){
				RecalcMatMul();
				GLUI_Master.sync_live_all();
			});
			spinner->set_speed(0.2f);
			//ele->set_w(215);
		}

		if (i != 2)
		{
			glui->add_column_to_panel(panel, false);
		}
	}
}

void ShowHelpWindow(int)
{
	static bool isHelpWindowOpen = false;
	static GLUI* helpWindow;
	if (!isHelpWindowOpen)
	{
		isHelpWindowOpen = true;
		helpWindow = GLUI_Master.create_glui("Help");
	}
	else
	{
		helpWindow->show();
		return;
	}

	helpWindow->add_statictext("This is a simple Euler Angle demonstration.");
	helpWindow->add_statictext("Set the rotation direction to Internal or External, then rotate the vector according to the specified rotation order.");
	helpWindow->add_statictext("You'll find the vector rotates around its own axes or the world axes respectively.");
	helpWindow->add_statictext("Author: huidong <mailhuid@163.com>");
	helpWindow->add_statictext("Date: 2025-06-30");
	helpWindow->add_statictext("Thanks to: FreeGLUT, GLUI");
	helpWindow->add_separator();
	helpWindow->add_button("OK", -1, (GLUI_CB)[](int){
		isHelpWindowOpen = false;
		helpWindow->close();
	});
}

// 創建矩陣整體的控制面板
void CreateMatrixSettingPanel(GLUI* glui)
{
	// 矩陣設置
	auto panel1 = glui->add_panel("", GLUI_PANEL_NONE);

	const char* pStrAxes[3] = { "X-Axis Rotation","Y-Axis Rotation","Z-Axis Rotation" };
	int pType[3] = { GLUI_TRANSLATION_X, GLUI_TRANSLATION_Y ,GLUI_TRANSLATION_Z };

	for (int i = 0; i < 3; i++)
	{
		auto trans = glui->add_translation_to_panel(panel1, pStrAxes[i], pType[i], &radianRotate[i], -1, (GLUI_CB)[](int){
			RecalcAll();

			// Temp
			float* vec1_r_ptr = vec1_r.host<float>();
			vecPath.push_back({ vec1_r_ptr[0], vec1_r_ptr[1], vec1_r_ptr[2] });
			af::freeHost(vec1_r_ptr);

			GLUI_Master.sync_live_all();
		});
		trans->set_speed(0.02f);
		glui->add_button_to_panel(panel1, "Reset", i, (GLUI_CB)[](int index){
			radianRotate[index] = 0;
			RecalcAll();
			GLUI_Master.sync_live_all();
		});

		if (i != 2)
		{
			glui->add_column_to_panel(panel1, false);
		}
	}

	// 一系列列式佈局控件
	auto panel2 = glui->add_panel("", GLUI_PANEL_NONE);

	// 立方體網格
	auto chk_wireframe = glui->add_checkbox_to_panel(panel2, "Show Wireframe", &doesShowWireframe, -1, (GLUI_CB)[](int){
		glutPostRedisplay();
	});

	// 運動軌跡
	auto chk_showpath = glui->add_checkbox_to_panel(panel2, "Show Path", &doesShowPath, -1, (GLUI_CB)[](int){
		glutPostRedisplay();
	});
	//panel2->set_alignment(GLUI_ALIGN_LEFT);
	//chk_wireframe->set_alignment(GLUI_ALIGN_LEFT);

	glui->add_column_to_panel(panel2, false);

	// 旋轉順序選擇
	auto rotationOrderCtrl = glui->add_listbox_to_panel(panel2, "Rotation Order", &rotationOrder, -1, (GLUI_CB)[](int){
		RecalcAll();
		GLUI_Master.sync_live_all();
	});
	const char* pStrRotationOrder[6] = {
		"X -> Y -> Z",
		"X -> Z -> Y",
		"Y -> X -> Z",
		"Y -> Z -> X",
		"Z -> X -> Y",
		"Z -> Y -> X"
	};
	for (int i = 0; i < 6; i++)
	{
		rotationOrderCtrl->add_item(i, pStrRotationOrder[i]);
	}
	//rotateOrderCtrl->set_alignment(GLUI_ALIGN_LEFT);

	auto rotationDirectionCtrl = glui->add_listbox_to_panel(panel2, "Rotation Direction", &rotationDirection, -1, (GLUI_CB)[](int){
		RecalcAll();
		GLUI_Master.sync_live_all();
	});
	rotationDirectionCtrl->add_item(ROTATE_INTERNAL, "Internal rotation");
	rotationDirectionCtrl->add_item(ROTATE_EXTERNAL, "External rotation");

	glui->add_column_to_panel(panel2, false);

	glui->add_button_to_panel(panel2, "Clear Path", -1, (GLUI_CB)[](int){
		vecPath.clear();
		glutPostRedisplay();
	});

	glui->add_button_to_panel(panel2, "Help", -1, ShowHelpWindow);
}

///////////////////////////////////
//	主函數和 GLUT、GLUI 的初始化
///////////////////////////////////

// 初始化OpenGL設置
void init()
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(windowWidth, windowHeight);
	main_window = glutCreateWindow("Math Playground | Euler Angle");

	init();

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutMouseWheelFunc(mouseWheel);
	glutKeyboardFunc(keyboard);

	/************** GLUI ***************/

	auto glui = GLUI_Master.create_glui("Control Panel");
	glui->set_main_gfx_window(main_window);
	
	auto panelCamera = glui->add_panel("Camera");
	glui->add_spinner_to_panel(panelCamera, "cameraRadianX", GLUI_SPINNER_FLOAT, &cameraRadianX);
	glui->add_spinner_to_panel(panelCamera, "cameraRadianY", GLUI_SPINNER_FLOAT, &cameraRadianY);
	
	glui->add_column();

	CreateMatrixValPanel(glui, "Mat1", arrMat[0]);
	CreateMatrixValPanel(glui, "Mat2", arrMat[1]);
	CreateMatrixValPanel(glui, "Mat3", arrMat[2]);
	CreateMatrixSettingPanel(glui);

	GLUI_Master.set_glutIdleFunc(idle);

	/************** End of GLUI ***************/

	// 進行一次初次計算
	RecalcAll();

	glutMainLoop();
	return 0;
}