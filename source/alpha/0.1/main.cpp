/*
zMand, Mandelbrot set viewer.
Copyright (C) 2014  Davide Zagami

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "zModule.h"
#include <cstdio>
#include <cstring>
#include <cmath>
#include <forward_list>

int SCREEN_WIDTH = 500;
int SCREEN_HEIGHT = 400;
double ASPECT_RATIO = ((double)SCREEN_WIDTH)/((double)SCREEN_HEIGHT);
const char W_TITLE[] = "zMand";

const double X_MIN0 = -2.4, Y_MIN0 = -1.5; //initial complex plane boundaries
const double VIEW_SPAN0 = 3.0; //initial complex plane view
const double MOVEMENT_FACTOR = 8.0;
const double ZOOM_FACTOR = 0.2;
const double RADIUS2 = 4.0;
const double PRECISION0 = 0.25;
const double log2_0 = log(2.0);

Uint8 colorschemeIndex = 0x00;
const Uint8 num_colorschemes = 7;
const int PALETTE_SIZE = 256;
SDL_Color insideColor[num_colorschemes];
SDL_Color palette1[PALETTE_SIZE];
SDL_Color palette2[PALETTE_SIZE];
SDL_Color palette3[PALETTE_SIZE];

double minX = X_MIN0, minY = Y_MIN0; //generic complex plane boundaries
double span = VIEW_SPAN0; //generic complex plane view
double precision = PRECISION0;
int n_threads = 1; //number of threads used to compute each mandelbrot set view
bool gauss = false;


bool linear_interpolation(SDL_Color& c, SDL_Color c1, SDL_Color c2, double t);
void createPalette();
void printInstructions();
bool init();
bool loadMedia();
void close();
int RenderMandelbrot(void* ptr);
void RenderLabels();
void RenderAll();
void MakeThreads(int w, int h);
void RenderZoomRect(int x0, int y0, int x1, int y1);
void MakeZoom(int x0, int y0, int x1, int y1);
void toggle_fullscreen();
void take_screenshot();
void save_screenshot(std::string filename);
void save_screenshot(const char* filename);

zWindow main_window;
SDL_Renderer* main_renderer = NULL;
SDL_Surface* screenSurface = NULL;
zTexture screenTexture;
zLabel labelTexture;
zLabel loadingTexture;



void gaussian_blur(SDL_Surface* surface)
{
	//point = SDL_MapRGBA(format, c.r, c.g, c.b, 0xFF);
	Uint32* pixels = (Uint32*)(surface->pixels); //Convert pixels to 32 bit
	Uint32 point_r = 0;
	Uint32 point_w = 0;
	Uint8 r = 0;
	Uint8 g = 0;
	Uint8 b = 0;
	Uint8 a = 0;
	SDL_PixelFormat* format = surface->format;
	Uint32 pitch = surface->pitch;
	Uint32 gauss_width = 2;
	Uint32 sumr = 0;
	Uint32 sumg = 0;
	Uint32 sumb = 0;
	Uint32 gauss_fact[] = {1,1};
	Uint32 gauss_sum = 2;

	/* orizzontal blurring */
	for(Uint32 i=1; i<surface->w-1; i++)
	{
		for(Uint32 j=1; j<surface->h-1; j++)
		{
			sumr = 0;
			sumg = 0;
			sumb = 0;
			for(Uint32 k=0; k<gauss_width; k++)
			{
				point_r = pixels[(j * (pitch/4)) + (i - ((gauss_width-1)>>1) + k)]; // reads pixel at (i - ((gauss_width-1)>>1) + k, j)
				SDL_GetRGBA(point_r, format, &r, &g, &b, &a); // reads rgba from read pixel
				sumr += ((Uint32)r)*gauss_fact[k];
				sumg += ((Uint32)g)*gauss_fact[k];
				sumb += ((Uint32)b)*gauss_fact[k];
			}
			point_w = SDL_MapRGBA(format, (Uint8)(sumr/gauss_sum), (Uint8)(sumg/gauss_sum), (Uint8)(sumb/gauss_sum), 0xFF); // gets resulting rgba
			pixels[j * (pitch/4) + i] = point_w; // and writes it at (i, j)
		}
	}
	/* vertical blurring */
	for(Uint32 i=1; i<surface->w-1; i++)
	{
		for(Uint32 j=1; j<surface->h-1; j++)
		{
			sumr = 0;
			sumg = 0;
			sumb = 0;
			for(Uint32 k=0; k<gauss_width; k++)
			{
				point_r = pixels[(((j - ((gauss_width-1)>>1) + k)%surface->h) * (pitch/4)) + i]; // reads pixel at (i, j - ((gauss_width-1)>>1) + k)
				SDL_GetRGBA(point_r, format, &r, &g, &b, &a); // reads rgba from read pixel
				sumr += ((Uint32)r)*gauss_fact[k];
				sumg += ((Uint32)g)*gauss_fact[k];
				sumb += ((Uint32)b)*gauss_fact[k];
			}
			point_w = SDL_MapRGBA(format, (Uint8)(sumr/gauss_sum), (Uint8)(sumg/gauss_sum), (Uint8)(sumb/gauss_sum), 0xFF); // gets resulting rgba
			pixels[j * (pitch/4) + i] = point_w; // and writes it at (i, j)
		}
	} 
}


// linear interpolates c1 and c2 with parameter t, saving the result in c
bool linear_interpolation(SDL_Color* c, SDL_Color c1, SDL_Color c2, double t)
{
	if(!((0.0<=t) && (t<=1.0))) // t must be between 0.0 and 1.0
		return false;
	c->r = (Uint8)((1.0-t)*c1.r + t*c2.r);
	c->g = (Uint8)((1.0-t)*c1.g + t*c2.g);
	c->b = (Uint8)((1.0-t)*c1.b + t*c2.b);
	return true;
}


void createPalette()
{
	// first palette
	for(int i=0; i<64; i++)
	{
		palette1[i] = {(Uint8)(4*i), (Uint8)(128-2*i), (Uint8)(255-4*i)};
	}
	for(int i=0; i<64; i++)
	{
		palette1[64+i] = {(Uint8)255, (Uint8)(4*i), (Uint8)0};
	}
	for(int i=0; i<64; i++)
	{
		palette1[128+i] = {(Uint8)(128-2*i), (Uint8)255, (Uint8)(4*i)};
	}
	for(int i=0; i<64; i++)
	{
		palette1[192+i] = {(Uint8)0, (Uint8)(255-4*i), (Uint8)(4*i)};
	}
	insideColor[0] = {0x00, 0x00, 0x00}; //black

	// second palette
	for(int i=0; i<64; i++)
	{
		palette2[i] = {(Uint8)(128-2*i), (Uint8)(255-4*i), (Uint8)0};
	}
	for(int i=0; i<64; i++)
	{
		palette2[64+i] = {(Uint8)(4*i), (Uint8)0, (Uint8)0};
	}
	for(int i=0; i<64; i++)
	{
		palette2[128+i] = {(Uint8)255, (Uint8)(4*i), (Uint8)0};
	}
	for(int i=0; i<64; i++)
	{
		palette2[192+i] = {(Uint8)(255-4*i), (Uint8)(4*i), (Uint8)0};
	}
	insideColor[1] = {0x00, 0x00, 0x00}; //black

	// third palette
	for(int i=0; i<64; i++)
	{
		palette3[i] = {(Uint8)(128+2*i), (Uint8)(4*i), (Uint8)255};
	}
	for(int i=0; i<64; i++)
	{
		palette3[64+i] = {(Uint8)(255-4*i), (Uint8)255, (Uint8)255};
	}
	for(int i=0; i<64; i++)
	{
		palette3[128+i] = {(Uint8)0, (Uint8)(255-4*i), (Uint8)255};
	}
	for(int i=0; i<64; i++)
	{
		palette3[192+i] = {(Uint8)(4*i), (Uint8)(255-4*i), (Uint8)255};
	}
	insideColor[2] = {0xFF, 0xFF, 0xFF}; //white

	insideColor[3] = {0x44, 0x00, 0x00}; //dark red

	insideColor[4] = {0x00, 0x44, 0x00}; //dark green

	insideColor[5] = {0x00, 0x00, 0x44}; //dark blue

	insideColor[6] = {0x44, 0x44, 0x44}; //dark grey
}


void printInstructions()
{
	fprintf(stdout, "<zMand 0.1 alpha>, Copyright (C) 2014  Davide Zagami\n");
	fprintf(stdout, "This is free software, and you are welcome to redistribute it under certain\n");
	fprintf(stdout, "conditions. See LICENSE.txt for details\n\n");
	fprintf(stdout, " 'ESC'    - Quit\n");
	fprintf(stdout, " 'WASD'   - Move view\n");
	fprintf(stdout, " 'Q'      - Zoom in\n");
	fprintf(stdout, " 'Z'      - Zoom out\n");
	fprintf(stdout, " 'LEFT'   - Go to previous view [TODO]\n");
	fprintf(stdout, " 'RIGHT'  - Go to successive view [TODO]\n");
	fprintf(stdout, " 'UP'     - Increase number of threads\n");
	fprintf(stdout, " 'DOWN'   - Decrease number of threads\n");
	fprintf(stdout, " 'R'      - Reset to standard view\n");
	fprintf(stdout, " 'X'      - Cicle to next color scheme\n");
	fprintf(stdout, " 'C'      - Cicle to previous color scheme\n");
	fprintf(stdout, " 'V'      - Reset to standard color scheme\n");
	fprintf(stdout, " 'T'      - Toggle gaussian blur\n");
	fprintf(stdout, " 'E'      - Take screenshot\n");
	fprintf(stdout, " 'F'      - Toggle Fullscreen (will ask for a change in resolution)\n");
	fprintf(stdout, " 'G'      - Change resolution\n");
	fprintf(stdout, "Drawing rectangles with mouse can also be used to change view.\n");
	fprintf(stdout, "The window can be resized and resolution will be changed accordingly.\n");
}


bool init()
{
	bool success = true;

	if(SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		fprintf(stderr, "SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		success = false;
	}
	else
	{
		//Set texture filtering to linear
		if(!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
			fprintf(stderr, "Warning: Linear texture filtering not enabled!\n");

		//Create window
		if(!main_window.init(W_TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE))
		{
			fprintf(stderr, "Window could not be created! SDL Error: %s\n", SDL_GetError());
			success = false;
		}
		else
		{
			main_renderer = main_window.createRenderer(-1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);
			if(main_renderer == NULL)
			{
				fprintf(stderr, "Renderer could not be created! SDL Error: %s\n", SDL_GetError());
				success = false;
			}
			else
			{
				SDL_SetRenderDrawColor(main_renderer, 0xFF, 0xFF, 0xFF, 0xFF);

				//Initialize SDL_ttf
				if(TTF_Init() == -1)
				{
					fprintf(stderr, "SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
					success = false;
				}

				/*
				//Initialize PNG, JPG and TIF loading
				int imgFlags = IMG_INIT_PNG|IMG_INIT_JPG|IMG_INIT_TIF;
				if( !(IMG_Init(imgFlags) & imgFlags) )
				{
					fprintf(stderr, "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
					success = false;
				}
				*/

				/*
				//Initialize FLAC, MOD, OGG and MP3 loading
				int mxrflags = MIX_INIT_FLAC|MIX_INIT_MOD|MIX_INIT_OGG|MIX_INIT_MP3;
				if( !(Mix_Init(mxrflags) & mxrflags) )
				{
					fprintf(stderr, "SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
					success = false;
				}
				*/
			}
		}
	}

	return success;
}


bool loadMedia()
{
	bool success = true;

	labelTexture.setFont("Sans.ttf", 12);
	loadingTexture.setFont("Sans.ttf", 24);

	labelTexture.setForegroundColor(0xFF, 0xFF, 0xFF, 0xFF);
	labelTexture.setBackgroundColor(0x00, 0x00, 0x00, 0xA0);
	loadingTexture.setForegroundColor(0xFF, 0xFF, 0xFF, 0xFF);
	loadingTexture.setBackgroundColor(0x00, 0x00, 0x00, 0xA0);

	loadingTexture.setText("RENDERING...");
	if(!loadingTexture.refresh(main_renderer))
	{
		fprintf(stderr, "Failed to render 'loading text' texture!\n");
		success = false;
	}

	return success;
}


void close()
{
	SDL_FreeSurface(screenSurface);
	screenSurface = NULL;
	screenTexture.free();
	labelTexture.free();
	loadingTexture.free();
	SDL_DestroyRenderer(main_renderer);
	main_renderer = NULL;
	main_window.free();
	TTF_Quit();
	//IMG_Quit();
	//Mixer_Quit();
	SDL_Quit();
}


int RenderMandelbrot(void* ptr)
{
	int* data = (int*)ptr;
	int s_width = data[0];
	int s_height = data[1];
	int begin = data[2];
	int end = data[3];
	Uint32* pixels = (Uint32*)(screenSurface->pixels); //Convert pixels to 32 bit
	Uint32 point = 0;
	SDL_PixelFormat* format = screenSurface->format;
	int pitch = screenSurface->pitch;
	double u, v, re, im, tempRe, modulus2, nu;
	precision = exp(log10(VIEW_SPAN0/span)/2.0); // sqrt of the exp of the base10 log of the current zoom factor makes sense, right?
	Uint32 maxIt = (Uint32)(PALETTE_SIZE*precision); // max iterations is proportional to the precision multiplier
	double spanfactor = span/s_height;
	SDL_Color c, c1, c2;
	Uint8 ii, iii;

	for(int y=begin; y<=end; y++)
	{
		for(int x=0; x<=s_width; x++)
		{
			u = minX + x*spanfactor;
			v = minY + y*spanfactor;
			re = u;
			im = v;
			tempRe = 0.0;
			c = insideColor[colorschemeIndex];
			for(Uint32 i=0; i<maxIt; i++)
			{
				tempRe = re*re - im*im + u;
				im = re*im*2.0 + v;
				re = tempRe;
				if((modulus2 = re*re + im*im) > RADIUS2)
				{
					// http://en.wikipedia.org/wiki/Mandelbrot_set#Continuous_.28smooth.29_coloring/
					nu = ((double)i) + 1.0 - log(0.5*log(modulus2)/log2_0)/log2_0;
					ii = ((int)nu)%PALETTE_SIZE;
					iii = ((int)nu+1)%PALETTE_SIZE;
					switch(colorschemeIndex)
					{	// coloring algorithms
						case 0:
							c1 = palette1[ii];
							c2 = palette1[iii];
							break;
						case 1:
							c1 = palette2[ii];
							c2 = palette2[iii];
							break;
						case 2:
							c1 = palette3[ii];
							c2 = palette3[iii];
							break;
						case 3:
							c1 = {ii, 0x00, 0x00};
							c2 = {iii, 0x00, 0x00};
							break;
						case 4:
							c1 = {0x00, ii, 0x00};
							c2 = {0x00, iii, 0x00};
							break;
						case 5:
							c1 = {0x00, 0x00, ii};
							c2 = {0x00, 0x00, iii};
							break;
						case 6:
							c1 = {ii, ii, ii};
							c2 = {iii, iii, iii};
							break;
					}
					linear_interpolation(&c, c1, c2, nu-floor(nu));
					break;
				}
			}
			point = SDL_MapRGBA(format, c.r, c.g, c.b, 0xFF);
			pixels[(y * (pitch/4)) + x] = point; //color selected pixel
		}
	}

	return 0;
}


void RenderLabels()
{
	char tempBuff[100];
	int dx=10, dy=10;

	//topleft corner label
	sprintf(tempBuff, "(%+1.16f, %+1.16f)", minX, minY);
	labelTexture.setText(tempBuff);
	if(!labelTexture.refresh(main_renderer))
	{
		fprintf(stderr, "Failed to render topleft corner label texture!\n");
	}
	else
	{
		labelTexture.topleft(dx, dy);
		labelTexture.render(main_renderer);
	}

	//bottomright corner label
	sprintf(tempBuff, "(%+1.16f, %+1.16f)", minX+span*ASPECT_RATIO, minY+span);
	labelTexture.setText(tempBuff);
	if(!labelTexture.refresh(main_renderer))
	{
		fprintf(stderr, "Failed to render bottomright corner label texture!\n");
	}
	else
	{
		labelTexture.bottomright(SCREEN_WIDTH-dx, SCREEN_HEIGHT-dy);
		labelTexture.render(main_renderer);
	}

	//Threads label
	sprintf(tempBuff, "Threads: %u", n_threads);
	labelTexture.setText(tempBuff);
	if(!labelTexture.refresh(main_renderer))
	{
		fprintf(stderr, "Failed to render threads label texture!\n");
	}
	else
	{
		labelTexture.topright(SCREEN_WIDTH-dx, dy);
		labelTexture.render(main_renderer);
	}

	//Zoom label
	sprintf(tempBuff, "Zoom: %e", VIEW_SPAN0/span);
	labelTexture.setText(tempBuff);
	if(!labelTexture.refresh(main_renderer))
	{
		fprintf(stderr, "Failed to render zoom label texture!\n");
	}
	else
	{
		labelTexture.bottomleft(dx, SCREEN_HEIGHT-dy);
		labelTexture.render(main_renderer);
	}

	//Precision label
	dy = labelTexture.getHeight()+2*dx;
	sprintf(tempBuff, "Precision: %f", precision);
	labelTexture.setText(tempBuff);
	if(!labelTexture.refresh(main_renderer))
	{
		fprintf(stderr, "Failed to render precision label texture!\n");
	}
	else
	{
		labelTexture.bottomleft(dx, SCREEN_HEIGHT-dy);
		labelTexture.render(main_renderer);
	}
}


void RenderAll()
{
	/* show 'RENDERING...' */
	loadingTexture.center_at(SCREEN_WIDTH/2, SCREEN_HEIGHT/2);
	loadingTexture.render(main_renderer);
	SDL_RenderPresent(main_renderer);

	/* prepare surface to render current mandelbrot view */
	SDL_SetRenderDrawColor(main_renderer, insideColor[colorschemeIndex].r, insideColor[colorschemeIndex].g, insideColor[colorschemeIndex].b, 0xFF);
	SDL_FreeSurface(screenSurface);
	screenSurface = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0, 0, 0, 0);
	if(screenSurface == NULL)
	{
		fprintf(stderr, "Unable to create new surface! SDL Error: %s\n", SDL_GetError());
		return;
	}

	MakeThreads(SCREEN_WIDTH, SCREEN_HEIGHT);
	if(gauss) gaussian_blur(screenSurface);

	/* show labels, render everything, and take a screenshot for later */
	screenTexture.loadFromSurface(main_renderer, screenSurface, false, insideColor[0], SDL_TEXTUREACCESS_TARGET);
	screenTexture.setAsRenderTarget(main_renderer);
	RenderLabels();
	SDL_SetRenderTarget(main_renderer, NULL);
	screenTexture.render(main_renderer);
	SDL_RenderPresent(main_renderer);
}


void MakeThreads(int w, int h)
{
	/* initialize threads to render current mandelbrot view */
	int data[n_threads][4];
	char threadname[10] = {0};
	std::forward_list<SDL_Thread*> thread_list;

	int t = h/n_threads;
	for(int i=0; i<n_threads-1; i++)
	{
		data[i][0] = w;
		data[i][1] = h;
		data[i][2] = i*t;
		data[i][3] = (i+1)*t-1;
		sprintf(threadname, "T%u", i);
		thread_list.emplace_front(SDL_CreateThread(RenderMandelbrot, threadname, (void*)data[i]));
	}
	data[n_threads-1][0] = w;
	data[n_threads-1][1] = h;
	data[n_threads-1][2] = (n_threads-1)*t;
	data[n_threads-1][3] = h;
	sprintf(threadname, "T%u", n_threads-1);
	thread_list.emplace_front(SDL_CreateThread(RenderMandelbrot, threadname, (void*)data[n_threads-1]));

	for(SDL_Thread* th : thread_list)
	{
		SDL_WaitThread(th, NULL);
	}
	thread_list.clear();
}


void RenderZoomRect(int x0, int y0, int x1, int y1)
{
	int spany = y1-y0;
	int spanx = (int)spany*ASPECT_RATIO;
	SDL_Rect ZoomRect = {x0-spanx, y0-spany, spanx*2, spany*2};
	screenTexture.render(main_renderer);
	SDL_SetRenderDrawColor(main_renderer, 0xFF, 0xFF, 0x00, 0xFF);
	SDL_RenderDrawRect(main_renderer, &ZoomRect);
	SDL_RenderPresent(main_renderer);
}


void MakeZoom(int x0, int y0, int x1, int y1)
{
	double ix = (double)x0;
	double iy = (double)y0;
	//double fx = (double)x1; // unused variable
	double fy = (double)y1;
	double spany = ((fy>=iy) ? (fy-iy) : (iy-fy));
	double spanx = spany*ASPECT_RATIO;
	ix = ix-spanx;
	iy = iy-spany;
	spanx *= 2.0;
	spany *= 2.0;

	minX = minX + ix*span/SCREEN_HEIGHT;
	minY = minY + iy*span/SCREEN_HEIGHT;
	span = spany*span/SCREEN_HEIGHT;
	RenderAll();
}


void toggle_fullscreen()
{
	if(main_window.isFullScreen())
		main_window.handleFullscreen(0);
	else
	{
		int s;
		fprintf(stdout, "\n\nUse maximum resolution on fullscreen? [y,n]\n");
		s = getchar();
		fflush(stdin);
		if((char)s == 'y')
		{
			main_window.handleFullscreen(SDL_WINDOW_FULLSCREEN_DESKTOP);
		}
		else if((char)s == 'n')
		{
			main_window.handleFullscreen(SDL_WINDOW_FULLSCREEN);
		}
		else
		{
			fprintf(stdout, "Canceled.\n");
		}
	}
}


void take_screenshot()
{
	char s[16] = {0};
	int w, h;
	fprintf(stdout, "Choose resolution for screenshot [WIDTHxHEIGHT]: ");
	fgets(s, 15, stdin);
	fflush(stdin);
	if((s[strlen(s)-1] == '\n'))
		s[strlen(s)-1] = '\0';
	sscanf(s, "%dx%d", &w, &h);

	/* prepare surface to render current mandelbrot view */
	SDL_SetRenderDrawColor(main_renderer, insideColor[colorschemeIndex].r, insideColor[colorschemeIndex].g, insideColor[colorschemeIndex].b, 0xFF);
	SDL_FreeSurface(screenSurface);
	screenSurface = SDL_CreateRGBSurface(0, w, h, 32, 0, 0, 0, 0);
	if(screenSurface == NULL)
	{
		fprintf(stderr, "Unable to create new surface! SDL Error: %s\n", SDL_GetError());
		return;
	}

	fprintf(stdout, "Apply gaussian blur? [y, n]\n");
	int g = getchar();
	fflush(stdin);
	fprintf(stdout, "Saving... ");
	MakeThreads(w, h);
	if((char)g == 'y')
	{
		gaussian_blur(screenSurface);
	}
}


void save_screenshot(std::string filename)
{
	save_screenshot(filename.c_str());
}


void save_screenshot(const char* filename)
{
	take_screenshot();
	if(SDL_SaveBMP(screenSurface, filename))
	{
		fprintf(stderr, "Unable to save screenshot! SDL Error: %s\n", SDL_GetError());
		return;
	}
	fprintf(stdout, "Screenshot saved to %s\n", filename);
}



int main(int argc, char* args[])
{
	if(!init())
		fprintf(stderr, "Failed to initialize!\n");
	else
	{
		if(!loadMedia())
			fprintf(stderr, "Failed to load media!\n");
		else
		{
			createPalette();
			printInstructions();
			RenderAll();
			bool quit = false, drawing_rect = false;
			int imx, imy, fmx, fmy;
			char s[256] = {0};
			SDL_Event e;
			while(!quit)
			{ //MAINLOOP BEGIN
				while(SDL_PollEvent(&e) != 0)
				{ //EVENTS BEGIN
					if(e.type == SDL_QUIT)
						quit = true;
					else if(e.type == main_window.getEventType())
					{
						SCREEN_WIDTH = main_window.getWidth();
						SCREEN_HEIGHT = main_window.getHeight();
						ASPECT_RATIO = ((double)SCREEN_WIDTH)/((double)SCREEN_HEIGHT);
						if(!loadingTexture.refresh(main_renderer))
						{
							fprintf(stderr, "Failed to render 'RENDERING...' texture!\n");
						}
						RenderAll();
					}
					else if(e.type == SDL_KEYDOWN)
					{ //KEYDOWN BEGIN
						switch(e.key.keysym.sym)
						{ //SWITCH KEYDOWN BEGIN
							case SDLK_ESCAPE:
								quit = true;
								break;

							case SDLK_w:
								minY -= span/MOVEMENT_FACTOR; //move camera up
								RenderAll();
								break;
							case SDLK_a:
								minX -= span*ASPECT_RATIO/MOVEMENT_FACTOR; //move camera left
								RenderAll();
								break;
							case SDLK_s:
								minY += span/MOVEMENT_FACTOR; //move camera down
								RenderAll();
								break;
							case SDLK_d:
								minX += span*ASPECT_RATIO/MOVEMENT_FACTOR; //move camera right
								RenderAll();
								break;

							case SDLK_q: //zoom in
								minX = (2.0*minX+(1.0-ZOOM_FACTOR)*span*ASPECT_RATIO)/2.0;
								minY = (2.0*minY+(1.0-ZOOM_FACTOR)*span)/2.0;
								span *= ZOOM_FACTOR;
								RenderAll();
								break;
							case SDLK_z: //zoom out
								minX = minX + span*ASPECT_RATIO*(1.0-1.0/ZOOM_FACTOR)/2.0;
								minY = minY + span*(1.0-1.0/ZOOM_FACTOR)/2.0;
								span /= ZOOM_FACTOR;
								RenderAll();
								break;

							case SDLK_r: //reset to standard view
								span = VIEW_SPAN0;
								minY = Y_MIN0;
								minX = X_MIN0;
								RenderAll();
								break;

							case SDLK_x: //increase colorschemeIndex
								if(++colorschemeIndex == num_colorschemes)
									colorschemeIndex = 0x00;
								RenderAll();
								break;
							case SDLK_c: //decrease colorschemeIndex
								if(colorschemeIndex == 0x00)
									colorschemeIndex = num_colorschemes;
								colorschemeIndex--;
								RenderAll();
								break;
							case SDLK_v: //reset to standard colorschemeIndex
								colorschemeIndex = 0x00;
								RenderAll();
								break;

							case SDLK_t: //toggle gaussian blur
								gauss = !gauss;
								RenderAll();
								break;

							case SDLK_f: //toggle fullscreen
								toggle_fullscreen();
								break;
							case SDLK_g: //change resolution
								fprintf(stdout, "\n\nCurrent resolution is %ux%u\n", SCREEN_WIDTH, SCREEN_HEIGHT);
								fprintf(stdout, "New resolution [WIDTHxHEIGHT]: ");
								fgets(s, 15, stdin);
								fflush(stdin);
								if((s[strlen(s)-1] == '\n'))
									s[strlen(s)-1] = '\0';
								main_window.setSize(s);
								break;

							case SDLK_e: //take screenshot
								fprintf(stdout, "\n\nBMP file path for screenshot (no spaces): ");
								fgets(s, 255, stdin);
								fflush(stdin);
								if((s[strlen(s)-1] == '\n'))
									s[strlen(s)-1] = '\0';
								save_screenshot(s);
								break;
								
							case SDLK_UP: //increase number of threads
								n_threads++;
								RenderAll();
								break;
							case SDLK_DOWN: //decrease number of threads
								n_threads = ((n_threads>1) ? (n_threads-1) : 1);
								RenderAll();
								break;
						} //SWITCH KEYDOWN END
					} //KEYDOWN END
					else if(e.type == SDL_MOUSEBUTTONDOWN)
					{
						drawing_rect = true;
						SDL_GetMouseState(&imx, &imy);
					}
					else if(e.type == SDL_MOUSEBUTTONUP)
					{
						drawing_rect = false;
						MakeZoom(imx, imy, fmx, fmy);
					}
					else
					{
						main_window.handleEvent(main_renderer, e);
					}
				} //EVENTS END

				if(drawing_rect)
				{
					SDL_GetMouseState(&fmx, &fmy);
					RenderZoomRect(imx, imy, fmx, fmy);
				}
			} //MAINLOOP END
		}
	}

	close();
	return 0;
}
