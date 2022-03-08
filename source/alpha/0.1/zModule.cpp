/*
zModule, simple graphical user interface engine based on SDL.
See zModule.h for info about copyright.
*/

#include "zModule.h"
#include <cstdio>
#include <cstring>




zWindow::zWindow()
{
	//Initialize non-existent window
	mWindow = NULL;
	mID = 0x00;
	mMouseFocus = false;
	mKeyboardFocus = false;
	mFullScreen = false;
	mMinimized = false;
	mWidth = 0;
	mHeight = 0;
}

zWindow::~zWindow()
{
	free();
}

bool zWindow::init(const char* title, int x, int y, int w, int h, Uint32 flags)
{
	mWindow = SDL_CreateWindow(title, x, y, w, h, flags);
	if(mWindow != NULL)
	{
		mID = SDL_GetWindowID(mWindow);
		Uint32 eventtype = SDL_RegisterEvents(1);
		if(eventtype != ((Uint32)-1))
		{
			SDL_zero(mEvent);
			mEvent.type = eventtype; // custom event type for this window
			mEvent.user.code = 1111;
			mEvent.user.data1 = NULL;
			mEvent.user.data2 = NULL;
		}
		mMouseFocus = true;
		mKeyboardFocus = true;
		mWidth = w;
		mHeight = h;
	}
	return mWindow != NULL;
}

bool zWindow::init(std::string title, int x, int y, int w, int h, Uint32 flags)
{
	return init(title.c_str(), x, y, w, h, flags);
}

SDL_Renderer* zWindow::createRenderer(int index, Uint32 flags)
{
	return SDL_CreateRenderer(mWindow, index, flags);
}

void zWindow::handleEvent(SDL_Renderer* renderer, SDL_Event& e)
{
	if(e.type == SDL_WINDOWEVENT)
	{ //WINDOW EVENT BEGIN
		if(e.window.windowID == mID)
		{
			switch(e.window.event)
			{ //SWITCH WINDOW EVENT BEGIN
				//Get new dimensions and repaint on window size change
				case SDL_WINDOWEVENT_SIZE_CHANGED:
					mWidth = e.window.data1;
					mHeight = e.window.data2;
					SDL_PushEvent(&mEvent);
					break;

				//Repaint on exposure
				case SDL_WINDOWEVENT_EXPOSED:
					SDL_RenderPresent(renderer);
					break;

				//Mouse entered window
				case SDL_WINDOWEVENT_ENTER:
					mMouseFocus = true;
					break;
				
				//Mouse left window
				case SDL_WINDOWEVENT_LEAVE:
					mMouseFocus = false;
					break;

				//Window has keyboard focus
				case SDL_WINDOWEVENT_FOCUS_GAINED:
					mKeyboardFocus = true;
					break;

				//Window lost keyboard focus
				case SDL_WINDOWEVENT_FOCUS_LOST:
					mKeyboardFocus = false;
					break;

				//Window minimized
				case SDL_WINDOWEVENT_MINIMIZED:
					mMinimized = true;
					break;

				//Window maxized
				case SDL_WINDOWEVENT_MAXIMIZED:
					mMinimized = false;
					break;
				
				//Window restored
				case SDL_WINDOWEVENT_RESTORED:
					mMinimized = false;
					break;
			} //SWITCH WINDOW EVENT END
		}
	} //WINDOW EVENT END
}

void zWindow::handleFullscreen(Uint32 flags)
{
	SDL_SetWindowFullscreen(mWindow, flags);
	mFullScreen = (bool)flags;
}

void zWindow::free()
{
	if(mWindow != NULL)
	{
		SDL_DestroyWindow(mWindow);
	}
	mWindow = NULL;
	mID = 0x00;
	mMouseFocus = false;
	mKeyboardFocus = false;
	mFullScreen = false;
	mMinimized = false;
	mWidth = 0;
	mHeight = 0;
}

Uint32 zWindow::getEventType()
{
	return mEvent.type;
}

int zWindow::getWidth()
{
	return mWidth;
}

int zWindow::getHeight()
{
	return mHeight;
}

bool zWindow::setSize(int width, int height)
{
	SDL_SetWindowSize(mWindow, width, height);
	return width && height; //if either is zero, SDL_SetWindowSize doesn't perform the resizing
}

bool zWindow::setSize(std::string s)
{
	return setSize(s.c_str());
}

bool zWindow::setSize(const char* s)
{
	int w, h;
	sscanf(s, "%dx%d", &w, &h);
	return setSize(w, h);
}

bool zWindow::hasMouseFocus()
{
	return mMouseFocus;
}

bool zWindow::hasKeyboardFocus()
{
	return mKeyboardFocus;
}

bool zWindow::isFullScreen()
{
	return mFullScreen;
}

bool zWindow::isMinimized()
{
	return mMinimized;
}



zBaseTexture::zBaseTexture()
{
	/* Initialize */
	mTexture = NULL;
	mWidth = 0;
	mHeight = 0;
	xpos = 0;
	ypos = 0;
}

zBaseTexture::~zBaseTexture()
{
	//Deallocate
}

void zBaseTexture::topleft(int x, int y)
{
	xpos = x;
	ypos = y;
}

void zBaseTexture::topright(int x, int y)
{
	xpos = x-mWidth;
	ypos = y;
}

void zBaseTexture::bottomleft(int x, int y)
{
	xpos = x;
	ypos = y-mHeight;
}

void zBaseTexture::bottomright(int x, int y)
{
	xpos = x-mWidth;
	ypos = y-mHeight;
}

void zBaseTexture::center_at(int x, int y)
{
	xpos = x - mWidth/2;
	ypos = y - mHeight/2;
}

int zBaseTexture::getxPos()
{
	return xpos;
}

int zBaseTexture::getyPos()
{
	return ypos;
}

void zBaseTexture::render(SDL_Renderer* renderer, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip)
{
	SDL_Rect renderQuad = {xpos, ypos, mWidth, mHeight}; //Set rendering space

	/* Set clip rendering dimensions */
	if(clip != NULL)
	{
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	SDL_RenderCopyEx(renderer, mTexture, clip, &renderQuad, angle, center, flip); //Render to screen
}

int zBaseTexture::getWidth()
{
	return mWidth;
}

int zBaseTexture::getHeight()
{
	return mHeight;
}




zTexture::zTexture()
{
	/* Initialize */
	mTexture = NULL;
	mWidth = 0;
	mHeight = 0;
	mPixels = NULL;
	mPitch = 0;
	xpos = 0;
	ypos = 0;
}

zTexture::~zTexture()
{
	free(); //Deallocate
}

void zTexture::free()
{
	/* Free texture if it exists */
	if(mTexture != NULL)
	{
		SDL_DestroyTexture(mTexture);
		mTexture = NULL;
		mWidth = 0;
		mHeight = 0;
		mPixels = NULL;
		mPitch = 0;
		xpos = 0;
		ypos = 0;
	}
}

bool zTexture::createBlank(SDL_Renderer* renderer, int width, int height, SDL_TextureAccess access)
{
	free(); //Get rid of preexisting texture

	/* Create uninitialized texture */
	mTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, access, width, height);
	if(mTexture == NULL)
	{
		fprintf(stderr, "Unable to create blank texture! SDL Error: %s\n", SDL_GetError());
	}
	else
	{
		mWidth = width;
		mHeight = height;
	}

	return mTexture != NULL;
}

bool zTexture::loadFromSurface(SDL_Renderer* renderer, SDL_Surface* surface, bool key, SDL_Color ckey, SDL_TextureAccess access)
{
	bool success = true;
	/* Convert surface to display format */
	SDL_Surface* formattedSurface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA8888, 0);
	if(formattedSurface == NULL)
	{
		fprintf(stderr, "Unable to convert surface to display format! SDL Error: %s\n", SDL_GetError());
		success = false;
	}
	else
	{
		/* Create blank texture */
		SDL_Texture* newTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, formattedSurface->w, formattedSurface->h);
		if(newTexture == NULL)
		{
			fprintf(stderr, "Unable to create blank texture! SDL Error: %s\n", SDL_GetError());
			success = false;
		}
		else
		{
			free(); //Deallocate old texture
			SDL_SetTextureBlendMode(newTexture, SDL_BLENDMODE_BLEND); //Enable blending on texture
			SDL_LockTexture(newTexture, &formattedSurface->clip_rect, &mPixels, &mPitch); //Lock texture for manipulation
			memcpy(mPixels, formattedSurface->pixels, formattedSurface->pitch * formattedSurface->h); //Copy formatted surface pixels
			// void * memcpy ( void * destination, const void * source, size_t num );
			// returns destination

			/* Get image dimensions */
			mWidth = formattedSurface->w;
			mHeight = formattedSurface->h;

			if(key) //if a colorkey was chosen
			{
				/* Get pixel data in editable format */
				Uint32* pixels = (Uint32*)mPixels; //Convert pixels to 32 bit
				int pixelCount = (mPitch/4)*mHeight; //mPitch/4 -> 4 bytes per pixel
				/* Map colors */
				Uint32 colorKey = SDL_MapRGB(formattedSurface->format, ckey.r, ckey.g, ckey.b);
				Uint32 transparent = SDL_MapRGBA(formattedSurface->format, 0x00, 0x00, 0x00, 0x00);
				/* Apply color key to pixels */
				for(int i = 0; i<pixelCount; ++i)
				{
					if(pixels[i] == colorKey)
					{
						pixels[i] = transparent;
					}
				}
			}

			SDL_UnlockTexture(newTexture); //Unlock texture to update
			mPixels = NULL; //Unlocked texture means mPixels pointer is no longer valid
			if(access != SDL_TEXTUREACCESS_STREAMING)
			{
				/* used to change access */
				mTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, access, formattedSurface->w, formattedSurface->h);
				SDL_Texture* original_target = SDL_GetRenderTarget(renderer);
				SDL_SetRenderTarget(renderer, mTexture);
				SDL_RenderCopy(renderer, newTexture, NULL, NULL); //copy pixels to texture with new access
				SDL_SetRenderTarget(renderer, original_target);
				SDL_DestroyTexture(newTexture); //destroy texture with old access
			}
			else
			{
				mTexture = newTexture; //Update with new texture
			}
		}
		SDL_FreeSurface(formattedSurface); //Get rid of old formatted surface
	}

	return success;
}

bool zTexture::loadFromFile(SDL_Renderer* renderer, std::string path, bool key, SDL_Color ckey, SDL_TextureAccess access)
{
	return loadFromFile(renderer, path.c_str(), key, ckey, access);
}

bool zTexture::loadFromFile(SDL_Renderer* renderer, const char* path, bool key, SDL_Color ckey, SDL_TextureAccess access)
{
	bool success = true;
	SDL_Surface* loadedSurface = IMG_Load(path); //Load image at specified path
	if(loadedSurface == NULL)
	{
		fprintf(stderr, "Unable to load image %s! SDL_image Error: %s\n", path, IMG_GetError());
		success = false;
	}
	else
	{
		success = loadFromSurface(renderer, loadedSurface, key, ckey, access);
		SDL_FreeSurface(loadedSurface); //Get rid of old loaded surface
	}

	return success;
}

int zTexture::setColorMod(Uint8 red, Uint8 green, Uint8 blue)
{
	return SDL_SetTextureColorMod(mTexture, red, green, blue);
}

int zTexture::setBlendMode(SDL_BlendMode blending)
{
	return SDL_SetTextureBlendMode(mTexture, blending);
}

int zTexture::setAlpha(Uint8 alpha)
{
	return SDL_SetTextureAlphaMod(mTexture, alpha);
}

int zTexture::setAsRenderTarget(SDL_Renderer* renderer)
{
	return SDL_SetRenderTarget(renderer, mTexture);
}

bool zTexture::lockTexture()
{
	bool success = true;

	if(mPixels != NULL)
	{
		fprintf(stderr, "Texture is already locked!\n");
		success = false;
	}
	else
	{
		if(SDL_LockTexture(mTexture, NULL, &mPixels, &mPitch) != 0) //NULL locks the entire texture, for now (should use an SDL_Rect)
		{
			fprintf(stderr, "Unable to lock texture! %s\n", SDL_GetError());
			success = false;
		}
	}

	return success;
}

bool zTexture::unlockTexture()
{
	bool success = true;

	if(mPixels == NULL)
	{
		printf("Texture is not locked!\n");
		success = false;
	}
	else
	{
		SDL_UnlockTexture(mTexture);
		mPixels = NULL;
		mPitch = 0;
	}

	return success;
}

void* zTexture::getPixels()
{
	return mPixels;
}

void zTexture::copyPixels(void* pixels)
{
	if(mPixels != NULL)
	{
		/* Texture is locked */
		memcpy(mPixels, pixels, mPitch * mHeight); //Copy to locked pixels
		// void * memcpy ( void * destination, const void * source, size_t num );
		// returns destination
	}
}

int zTexture::getPitch()
{
	return mPitch;
}

Uint32 zTexture::getPixel_at(unsigned int x, unsigned int y)
{
	if(mPixels != NULL)
	{
		/* Texture is locked */
		Uint32* pixels = (Uint32*)mPixels; //Convert pixels to 32 bit
		return pixels[(y * (mPitch/4)) + x]; //Get the requested pixel (mPitch/4 -> 4 bytes per pixel)
	}
}




zLabel::zLabel()
{
	mTexture = NULL;
	mFont = NULL;
	mFColor = {0,0,0,0};
	mBColor = {0,0,0,0};
	mText = "";
	mWidth = 0;
	mHeight = 0;
	xpos = 0;
	ypos = 0;
}

zLabel::~zLabel()
{
	free();
}

void zLabel::free()
{
	freeTexture();
	freeFont();
}

void zLabel::freeTexture()
{
	if(mTexture != NULL)
	{
		SDL_DestroyTexture(mTexture);
		mTexture = NULL;
		mWidth = 0;
		mHeight = 0;
		xpos = 0;
		ypos = 0;
	}
}

void zLabel::freeFont()
{
	if(mFont != NULL)
	{
		TTF_CloseFont(mFont);
		mFont = NULL;
	}
}

bool zLabel::setFont(std::string path, int size)
{
	return setFont(path.c_str(), size);
}

bool zLabel::setFont(const char* path, int size)
{
	bool success = true;
	freeFont();
	mFont = TTF_OpenFont(path, size);
	if(mFont == NULL)
	{
		fprintf(stderr, "Failed to load font! SDL_ttf Error: %s\n", TTF_GetError());
		success = false;
	}
	return success;
}

void zLabel::setForegroundColor(Uint8 red, Uint8 green, Uint8 blue, Uint8 alpha)
{
	mFColor = {red, green, blue, alpha};
}

void zLabel::setBackgroundColor(Uint8 red, Uint8 green, Uint8 blue, Uint8 alpha)
{
	mBColor = {red, green, blue, alpha};
}

void zLabel::setText(std::string text)
{
	mText = text;
}

void zLabel::setText(const char* text)
{
	mText = text;
}

bool zLabel::refresh(SDL_Renderer* renderer)
{
	freeTexture(); //Get rid of preexisting texture
	SDL_Surface* textSurface = TTF_RenderText_Blended(mFont, mText.c_str(), mFColor);
	if(textSurface == NULL)
	{
		fprintf(stderr, "Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
	}
	else
	{
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
		if(textTexture == NULL)
		{
			fprintf(stderr, "Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError());
		}
		else
		{
			mWidth = textSurface->w;
			mHeight = textSurface->h;
			//Create uninitialized texture
			mTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, mWidth+10, mHeight);
			if(mTexture == NULL)
			{
				fprintf(stderr, "Unable to create blank texture! SDL Error: %s\n", SDL_GetError());
			}
			else
			{
				SDL_Rect renderQuad = {5, 0, mWidth, mHeight};
				SDL_SetTextureBlendMode(mTexture, SDL_BLENDMODE_BLEND);
				SDL_Texture* original_target = SDL_GetRenderTarget(renderer);
				SDL_SetRenderTarget(renderer, mTexture);
				SDL_SetRenderDrawColor(renderer, mBColor.r, mBColor.g, mBColor.b, mBColor.a);
				SDL_RenderClear(renderer);
				SDL_RenderCopyEx(renderer, textTexture, NULL, &renderQuad, 0.0, NULL, SDL_FLIP_NONE);
				SDL_SetRenderTarget(renderer, original_target);
			}
			SDL_DestroyTexture(textTexture); //Get rid of old texture
		}
		SDL_FreeSurface(textSurface); //Get rid of old surface
	}
	return mTexture != NULL;
}



