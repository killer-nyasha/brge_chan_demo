#pragma once

using namespace System;
using namespace System;
using namespace System::Drawing;
using namespace System::Drawing::Imaging;
using namespace System::Runtime::InteropServices;
using namespace System::Windows::Forms;
using namespace System::ComponentModel;
using namespace System::Collections::Generic;

using byte = unsigned char;

#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <queue>
#include <map>

#include "C:\Users\Isstrebitel\git\BEST-RPG-GAME-EVER\BestRpgGameEver\SstpContent\vectors.h"

#include "..\LizardScript\LizardScriptCompiler.h"
#include "..\LizardScript\Runtime.h"
#include "..\LizardScript\LizardScriptLibrary.h"
#include "..\LizardScript\metagen.h"

//#pragma pack(push, 1)
struct PixelData
{
	byte b;
	byte g;
	byte r;
	byte a;
};
using Pixel = vect_prototype2<byte, 4, PixelData>;

struct LsPixel
{
	int r;
	int g;
	int b;
	int a;

	int x;
	int y;

	static void meta()
	{
		METAGEN_CLASS(LsPixel)
			WITH_MEMBERS(
				,FIELD(r)
				,FIELD(g)
				,FIELD(b)
				,FIELD(a)
				,FIELD(x)
				,FIELD(y)
				//,FIELD(width)
				//,FIELD(height)
				//,PARAMS(int, int)::FUNC(pixel)
			);
	}
};
//#pragma pack(pop)


namespace AttachmentsProcessor 
{
	enum LayerMode
	{
		LAYER_NORMAL = 0,
		LAYER_LINEAR_DARK = 1,
		LAYER_MUL = 2
	};

	public ref class Layer
	{
		Bitmap^ source;
		BitmapData^ sourceData;
		static BitmapData^ tempData;

		Pixel* sdata;
		Pixel* ddata;
		Pixel* tdata;

		int stride;
		int width, height;


		LizardScript::TypedExpr<LsPixel>* pixelShader = nullptr;

	public:

		String^ tag;

		int layerMode = LAYER_NORMAL;
		int fullOpacity = 100;

		bool static_initialized = false;
		void static_init()
		{
			LizardScript::standartCompiler = new LizardScript::LizardScriptCompiler(LizardScript::defaultSyntaxCore);
			LizardScript::lsl.printRunTime = false;
			LizardScript::lsl.printCompilationTime = false;
			LsPixel::meta();
		}

		Layer(Bitmap^ source, String^ tag, int layerMode, int opacity) : source(source), tag(tag), layerMode(layerMode), fullOpacity(opacity)
		{
			if (!static_initialized)
				static_init(), static_initialized = true;

			//Console::Write(tag);
			//std::cout << "; " << layerMode << std::endl;

			sourceData = source->LockBits(Rectangle(0, 0, source->Width, source->Height), ImageLockMode::ReadWrite, source->PixelFormat);
			sdata = (Pixel*)(void*)sourceData->Scan0;

			stride = sourceData->Stride;
			width = sourceData->Width;
			height = sourceData->Height;

			ddata = new Pixel[width * height]; //(Pixel*)(void*)destData->Scan0;
		}

		void staticUpdate(BitmapData^ tempData)
		{
			this->tempData = tempData;
			tdata = (Pixel*)(void*)tempData->Scan0;
			//clear();
		}

		void update(bool isFirst)
		{
			tdata = (Pixel*)(void*)tempData->Scan0;
			shader();
			apply(isFirst);
		}

		Pixel& s(int x, int y)
		{
			return /*sdata[x + width * y];*/*(Pixel*)&((byte*)sdata)[x * 4 + stride * y];
		}

		Pixel& d(int x, int y)
		{
			return /*ddata[x + width * y];*/*(Pixel*)&((byte*)ddata)[x * 4 + stride * y];
		}

		Pixel& t(int x, int y)
		{
			return /*tdata[x + width * y];*/*(Pixel*)&((byte*)tdata)[x * 4 + stride * y];
		}

		bool isShaderActual = false;

		void setShader(String^ text)
		{
			if (String::IsNullOrEmpty(text))
			{
				pixelShader = nullptr;
			}
			else
			{
				char* ch = new char[text->Length + 1];
				for (int i = 0; i < text->Length; i++)
					ch[i] = (char)text[i];
				ch[text->Length] = 0;
				pixelShader = new TypedExpr<LsPixel>(script<LsPixel>(ch));
				delete[] ch;
			}

			isShaderActual = false;
		}

		void shader()
		{
			if (!isShaderActual)
			{
				isShaderActual = true;

				if (pixelShader != nullptr)
					for (int y = 0; y < height; y++)
						for (int x = 0; x < width; x++)
						{
							LsPixel p;
							p.x = x;
							p.y = y;
							p.r = s(x, y).r;
							p.g = s(x, y).g;
							p.b = s(x, y).b;
							p.a = s(x, y).a;

							auto runtime = LizardScript::Runtime(*pixelShader, p);

							if (p.r > 255)
								p.r = 255;
							if (p.g > 255)
								p.g = 255;
							if (p.b > 255)
								p.b = 255;

							if (p.r < 0)
								p.r = 0;
							if (p.g < 0)
								p.g = 0;
							if (p.b < 0)
								p.b = 0;

							d(x, y).r = p.r;
							d(x, y).g = p.g;
							d(x, y).b = p.b;
							d(x, y).a = p.a;
						}
				else
					for (int y = 0; y < height; y++)
						for (int x = 0; x < width; x++)
							d(x, y) = s(x, y);
			}
		}

		void apply(bool isFirst)
		{
			if (isFirst)
			{
				for (int y = 0; y < height; y++)
					for (int x = 0; x < width; x++)
					{
						t(x, y) = d(x, y);
						t(x, y).a = 255;
					}
			}
			else
			for (int y = 0; y < height; y++)
				for (int x = 0; x < width; x++)
				{
					if (d(x, y).a == 0)
						continue;

					double opacity = ((double)fullOpacity / 100) * ((double)d(x, y).a / 255);

					if (layerMode == LAYER_NORMAL)
					{
						double r = (double)d(x, y).r * opacity + (double)t(x, y).r * (1.0 - opacity);
						double g = (double)d(x, y).g * opacity + (double)t(x, y).g * (1.0 - opacity);
						double b = (double)d(x, y).b * opacity + (double)t(x, y).b * (1.0 - opacity);

						t(x, y).r = r;
						t(x, y).g = g;
						t(x, y).b = b;
					}
					else if (layerMode == LAYER_LINEAR_DARK)
					{
						double r = (double)t(x, y).r - (double)(255 - d(x, y).r) * opacity;
						double g = (double)t(x, y).g - (double)(255 - d(x, y).g) * opacity;
						double b = (double)t(x, y).b - (double)(255 - d(x, y).b) * opacity;

						if (r < 0)
							r = 0;
						if (g < 0)
							g = 0;
						if (b < 0)
							b = 0;

						t(x, y).r = r;
						t(x, y).g = g;
						t(x, y).b = b;

					}
					else if (layerMode == LAYER_MUL)
					{
						double r = (double)t(x, y).r/255 * (double)(1.0 - (1.0 - (double)d(x, y).r / 255) * opacity);
						double g = (double)t(x, y).g/255 * (double)(1.0 - (1.0 - (double)d(x, y).g / 255) * opacity);
						double b = (double)t(x, y).b/255 * (double)(1.0 - (1.0 - (double)d(x, y).b / 255) * opacity);

						t(x, y).r = r*255;
						t(x, y).g = g*255;
						t(x, y).b = b*255;
					}
				}
		}
	};
}
