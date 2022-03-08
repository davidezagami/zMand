zMand v0.1 alpha
=======================================
https://sourceforge.net/projects/zmand/
zahamiedivad@hotmail.it


COMPILING THE PROGRAM:

	Compile with gcc, standard C++11.
	A text file with the correct command line to issue is included with
	the package.
	The program should work cross-platform.
	Tested only on Windows 32bit, for now.


DESCRIPTION:

	A dinamyc Mandelbrot set viewer written in C++/SDL2.
	Supports zooming, panning, different color schemes, arbitrary
	resolutions, multi-threading, saving screenshots as BMP.
	More features to come.


TO DO LIST:

	- Make zModule.h an independent GUI library (it already is, sort of,
	  but it's almost useless).
	
	- Graphical user interface instead of ugly console interaction.
	
	- Unlimited zooming.
	
	- Enable real time zooming/rendering with shaders (may require OpenGl
	  explicitly, which i want to avoid).
	
	- Enable view history, undo/redo operations.
	
	- Clean up spaghetti code.


CHANGES:

	v0.1:		- First release.
	
				- Supports zooming, panning, different color schemes,
				  arbitrary resolutions, multi-threading, saving
				  screenshots as BMP.


INFO ABOUT COPYRIGHT:

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
