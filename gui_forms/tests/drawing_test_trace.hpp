#pragma once

#include <string_view>

inline constexpr std::string_view gui_drawing_expected_trace =
    "gui.drawing.trace/v1\n"
    "0|clear|color=ff000000|transform=1,0,0,1,0,0|clip=none|quality=0,0,0,0,0\n"
    "1|save|token=1|transform=1,0,0,1,0,0|clip=none|quality=0,0,0,0,0\n"
    "2|translate|offset=10,5|transform=1,0,0,1,10,5|clip=none|quality=0,0,0,0,0\n"
    "3|set_clip|rect=0,0,100,50|transform=1,0,0,1,10,5|clip=10,5,100,50|quality=0,0,0,0,0\n"
    "4|set_quality|transform=1,0,0,1,10,5|clip=10,5,100,50|quality=4,5,4,0,2\n"
    "5|fill_rectangle|rect=1,2,30,20|brush=ffff0000|transform=1,0,0,1,10,5|clip=10,5,100,50|quality=4,5,4,0,2\n"
    "6|draw_rectangle|rect=1,2,30,20|pen=ffffff00,2,1|transform=1,0,0,1,10,5|clip=10,5,100,50|quality=4,5,4,0,2\n"
    "7|draw_line|from=1,2|to=31,22|pen=ffffff00,2,1|transform=1,0,0,1,10,5|clip=10,5,100,50|quality=4,5,4,0,2\n"
    "8|draw_string|at=4,7|font=Lucida Grande,12,1|brush=ffff0000|format=1,0,3,4|text=field \xce\xa9|transform=1,0,0,1,10,5|clip=10,5,100,50|quality=4,5,4,0,2\n"
    "9|restore|token=1|transform=1,0,0,1,0,0|clip=none|quality=0,0,0,0,0\n"
    "end|commands=10|closed=1\n";
