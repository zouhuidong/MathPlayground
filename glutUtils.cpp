#include "glutUtils.h"
#include <gl/freeglut.h>

// GLUT 筆畫字體的設計高度爲 100 單位，這是字體的固有屬性。
static void* g_glutStrokeFont = (void*)GLUT_STROKE_ROMAN;
static void* g_glutBitmapFont = (void*)GLUT_BITMAP_8_BY_13;

static float g_glutStrokeFontSize = 0.002f; // 預設筆畫字體大小

namespace glutUtils
{

	void setStrokeFont(void* font)
	{
		g_glutStrokeFont = font;
	}

	void setBitmapFont(void* font)
	{
		g_glutBitmapFont = font;
	}

	void setStrokeFontSize(float size)
	{
		g_glutStrokeFontSize = size * 0.01f;
	}

	void renderStrokeFontString(
		float x,
		float y,
		float z,
		const char* string,
		float dgree_x,
		float dgree_y,
		float dgree_z)
	{

		const char* c;
		glPushMatrix();
		glTranslatef(x, y, z);
		glRotatef(dgree_x, 1.0f, 0.0f, 0.0f);
		glRotatef(dgree_y, 0.0f, 1.0f, 0.0f);
		glRotatef(dgree_z, 0.0f, 0.0f, 1.0f);
		glScalef(g_glutStrokeFontSize, g_glutStrokeFontSize, g_glutStrokeFontSize);
		for (c = string; *c != '\0'; c++)
		{
			glutStrokeCharacter(g_glutStrokeFont, *c);
		}
		glPopMatrix();
	}

	void renderBitmapString(
		float x,
		float y,
		float z,
		const char* string)
	{
		// glRasterPos3f 是 OpenGL 中的一個函數，用於指定光柵化操作的位置。
		// 光柵化是將幾何圖元（如點、線、多邊形）轉換爲像素的過程，
		// 而 glRasterPos3f 則設置了後續像素操作（如繪製位圖、圖像或複製像素）的起始位置。
		//
		// 匯東註：在 Bitmap 文本是 2D 渲染，因此需要使用 glRasterPos3f 設置當前光柵位置爲指定的三維座標 (x, y, z)。
		// 如果使用 glTranslatef、glScalef 之類的函數，是不起作用的。
		//
		glRasterPos3f(x, y, z); // Set the position for bitmap text rendering
		const char* c;
		for (c = string; *c != '\0'; c++)
		{
			glutBitmapCharacter(g_glutBitmapFont, *c);
		}
	}

	// Set orthographic projection for 2D rendering
	void setOrthographicProjection()
	{
		// switch to projection mode
		glMatrixMode(GL_PROJECTION);

		// save previous matrix which contains the
		//settings for the perspective projection
		glPushMatrix();

		// reset matrix
		glLoadIdentity();

		// set a 2D orthographic projection
		gluOrtho2D(0, glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT), 0);

		// switch back to modelview mode
		glMatrixMode(GL_MODELVIEW);
	}

	// Set perspective projection for 3D rendering
	void restorePerspectiveProjection()
	{

		glMatrixMode(GL_PROJECTION);
		// restore previous projection matrix
		glPopMatrix();

		// get back to modelview mode
		glMatrixMode(GL_MODELVIEW);
	}
}

