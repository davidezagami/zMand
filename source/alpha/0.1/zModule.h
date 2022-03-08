/*
zModule, simple graphical user interface engine based on SDL.
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

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <string>



/* window class wrapper */
class zWindow
{
	public:
		//Intializes internals
		zWindow();

		//Deallocates memory
		~zWindow();

		//Creates window
		bool init(const char* title, int x, int y, int w, int h, Uint32 flags);
		bool init(std::string title, int x, int y, int w, int h, Uint32 flags);

		//Creates renderer from internal window
		SDL_Renderer* createRenderer(int index, Uint32 flags);

		//Handles window events
		void handleEvent(SDL_Renderer* renderer, SDL_Event& e);

		//Handles fullscreen
		void handleFullscreen(Uint32 flags);

		//Deallocates internals
		void free();

		//Get custom event signaler
		Uint32 getEventType();

		//Window dimensions
		int getWidth();
		int getHeight();

		bool setSize(int width, int height);
		bool setSize(std::string s);
		bool setSize(const char* s);

		//Window focii
		bool hasMouseFocus();
		bool hasKeyboardFocus();
		bool isFullScreen();
		bool isMinimized();

	private:
		//Window data
		SDL_Window* mWindow;
		Uint32 mID;
		SDL_Event mEvent;

		//Window dimensions
		int mWidth;
		int mHeight;

		//Window focii
		bool mMouseFocus;
		bool mKeyboardFocus;
		bool mFullScreen;
		bool mMinimized;
};



/* texture baseclass wrapper */
class zBaseTexture
{
	public:
		//Initializes variables
		zBaseTexture();

		//Deallocates memory
		~zBaseTexture();

		//Set topleft corner
		void topleft(int x, int y);

		//Set topright corner
		void topright(int x, int y);

		//Set bottomleft corner
		void bottomleft(int x, int y);

		//Set bottomright corner
		void bottomright(int x, int y);

		//Set center
		void center_at(int x, int y);

		//Get x component of topleft corner
		int getxPos();

		//Get y component of topleft corner
		int getyPos();

		//Renders texture at given point
		void render(SDL_Renderer* renderer, SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE);

		//Gets image dimensions
		int getWidth();
		int getHeight();

	protected:
		//The actual hardware texture
		SDL_Texture* mTexture;

		//Image dimensions
		int mWidth;
		int mHeight;

		//Image position (topleft corner)
		int xpos;
		int ypos;
};



/* generic texture class wrapper */
class zTexture : public zBaseTexture
{
	public:
		//Initializes variables
		zTexture();

		//Deallocates memory
		~zTexture();

		//Deallocates texture
		void free();

		//Creates blank texture
		bool createBlank(SDL_Renderer* renderer, int width, int height, SDL_TextureAccess access = SDL_TEXTUREACCESS_STREAMING);

		//Loads from specified surface
		bool loadFromSurface(SDL_Renderer* renderer, SDL_Surface* surface, bool key, SDL_Color ckey, SDL_TextureAccess access = SDL_TEXTUREACCESS_STREAMING);

		//Loads image at specified path
		bool loadFromFile(SDL_Renderer* renderer, std::string path, bool key, SDL_Color ckey, SDL_TextureAccess access = SDL_TEXTUREACCESS_STREAMING);
		bool loadFromFile(SDL_Renderer* renderer, const char* path, bool key, SDL_Color ckey, SDL_TextureAccess access = SDL_TEXTUREACCESS_STREAMING);

		//Set color modulation
		int setColorMod(Uint8 red, Uint8 green, Uint8 blue);

		//Set blending
		int setBlendMode(SDL_BlendMode blending);

		//Set alpha modulation
		int setAlpha(Uint8 alpha);

		//Set self as render target
		int setAsRenderTarget(SDL_Renderer* renderer);

		//Pixel manipulators
		bool lockTexture();
		bool unlockTexture();
		void* getPixels();
		void copyPixels(void* pixels);
		int getPitch();
		Uint32 getPixel_at(unsigned int x, unsigned int y);

	private:
		//Used for pixel manipulation
		void* mPixels;
		int mPitch;
};




/* rendered text class wrapper */
class zLabel : public zBaseTexture
{
	public:
		//Initializes variables
		zLabel();

		//Deallocates memory
		~zLabel();

		//Deallocates texture and font
		void free();

		//Deallocates texture
		void freeTexture();

		//Deallocates font
		void freeFont();

		//Set Font
		bool setFont(std::string path, int size);
		bool setFont(const char* path, int size);

		//Set foreground color
		void setForegroundColor(Uint8 red, Uint8 green, Uint8 blue, Uint8 alpha = 0XFF);

		//Set background color
		void setBackgroundColor(Uint8 red, Uint8 green, Uint8 blue, Uint8 alpha = 0XFF);

		//Set text
		void setText(std::string text);
		void setText(const char* text);

		//Refreshes texture
		bool refresh(SDL_Renderer* renderer);

	private:
		//Text properties
		TTF_Font* mFont;
		SDL_Color mFColor;
		SDL_Color mBColor;
		std::string mText;
};
