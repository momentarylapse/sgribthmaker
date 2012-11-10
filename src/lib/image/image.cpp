#include "image.h"
#include "../file/file.h"


string ImageVersion = "0.2.2.0";

void Image::__init__()
{
	data.__init__();
}

void Image::__delete__()
{
	data.clear();
}

// mode: rgba
//    = r + g<<8 + b<<16 + a<<24
void Image::LoadFlipped(const string &filename)
{
	msg_db_r("Image.LoadFlipped", 1);

	// reset image
	width = 0;
	height = 0;
	error = false;
	alpha_used = false;
	data.clear();

	// file ok?
	if (!file_test_existence(filename)){
		msg_error("Image.Load: file does not exist: " + filename);
		msg_db_l(1);
		return;
	}
	
	string ext = filename.extension();
	
	if (ext == "bmp")
		image_load_bmp(filename, *this);
	else if (ext == "tga")
		image_load_tga(filename, *this);
	else if (ext == "jpg")
		image_load_jpg(filename, *this);
	else
		msg_error("ImageLoad: unhandled file extension: " + ext);
	
	msg_db_l(1);
}

void Image::Load(const string &filename)
{
	LoadFlipped(filename);
	FlipV();
}

unsigned int image_color(const color &c)
{
	return (int(c.a * 255.0f) << 24) + (int(c.b * 255.0f) << 16) + (int(c.g * 255.0f) << 8) + int(c.r * 255.0f);
}

color image_uncolor(unsigned int i)
{
	return color((float)(i >> 24) / 255.0f,
	             (float)(i & 255) / 255.0f,
	             (float)((i >> 8) & 255) / 255.0f,
	             (float)((i >> 16) & 255) / 255.0f);
}

void Image::Create(int _width, int _height, const color &c)
{
	msg_db_r("Image.Create", 1);
	if (data.element_size != 4)
		data.init(4); // script...

	// create
	width = _width;
	height = _height;
	error = false;
	alpha_used = (c.a != 1.0f);
	
	// fill image
	data.resize(width * height);
	unsigned int ic = image_color(c);
	for (int i=0;i<data.num;i++)
		data[i] = ic;
	
	msg_db_l(1);
}

void Image::Save(const string &filename) const
{
	msg_db_r("Image.Save", 1);
	
	string ext = filename.extension();
	if (ext == "tga")
		image_save_tga(filename, *this);
	else
		msg_error("ImageSave: unhandled file extension: " + ext);
	
	msg_db_l(1);
}

void Image::Delete()
{
	msg_db_r("Image.Delete", 1);
	data.clear();
	msg_db_l(1);
}

void Image::Scale(int _width, int _height)
{
	msg_db_r("Image.Scale", 1);
	Array<unsigned int> _data;
	_data.resize(_width * _height);

	for (int x=0;x<_width;x++)
		for (int y=0;y<_height;y++){
			int x0 = (int)( (float)x * (float)width / (float)_width );
			int y0 = (int)( (float)y * (float)height / (float)_height );
			_data[y * _width + x] = data[y0 * width + x0];
		}

	data.exchange(_data);
	width = _width;
	height = _height;
	
	msg_db_l(1);
}

void Image::FlipV()
{
	msg_db_r("Image.FlipV", 1);
	
	unsigned int t;
	unsigned int *d = &data[0];
	for (int y=0;y<height/2;y++)
		for (int x=0;x<width;x++){
			t = d[x + y * width];
			d[x + y * width] = d[x + (height - y - 1) * width];
			d[x + (height - y - 1) * width] = t;
		}
	
	msg_db_l(1);
}

void Image::CopyTo(Image &dest) const
{
	msg_db_r("Image.Copy", 1);

	dest.width = width;
	dest.height = height;
	dest.data = data;
	dest.alpha_used = alpha_used;
	dest.error = error;

	msg_db_l(1);
}


void Image::SetPixel(int x, int y, const color &c)
{
	if ((x >= 0) and (x < width) and (y >= 0) and (y < height))
		data[x + y * width] = image_color(c);
}

color Image::GetPixel(int x, int y) const
{
	if ((x >= 0) and (x < width) and (y >= 0) and (y < height))
		return image_uncolor(data[x + y * width]);
	return Black;
}

// use bilinear interpolation
//  x repeats in [0..width)
//  y repeats in [0..height)
//  each pixel has its maximum at offset (0.5, 0.5)
color Image::GetPixelInterpolated(float x, float y) const
{
	x = loopf(x, 0, width);
	y = loopf(y, 0, height);
	int x0 = (x >= 0.5f) ? (x - 0.5f) : (width - 1);
	int x1 = (x0 < width - 1) ? (x0 + 1) : 0;
	int y0 = (y >= 0.5f) ? (y - 0.5f) : (height - 1);
	int y1 = (y0 < height- 1) ? (y0 + 1) : 0;
	color c00 = GetPixel(x0, y0);
	color c01 = GetPixel(x0, y1);
	color c10 = GetPixel(x1, y0);
	color c11 = GetPixel(x1, y1);
	float sx = loopf(x + 0.5f, 0, 1);
	float sy = loopf(y + 0.5f, 0, 1);
	// bilinear interpolation
	return (c00 * (1 - sy) + c01 * sy) * (1 - sx) + (c10 * (1 - sy) + c11 * sy) * sx;
}
