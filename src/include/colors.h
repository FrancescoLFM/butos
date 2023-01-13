#ifndef COLORS_H
#define COLORS_H

#define BACKGROUND(col) ((col) << 4)
#define FOREGROUND(col) (col)
#define COLOR(fg, bg)   (FOREGROUND(fg) | BACKGROUND(bg))
#define RED             0b100
#define GREEN           0b010
#define BLUE            0b001
#define WHITE           0b111
#define BLACK           0b000

#define STD_COLOR  0b00000111

#endif