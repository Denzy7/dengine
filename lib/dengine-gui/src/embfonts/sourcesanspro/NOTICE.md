# generation and changes
The font sources are generated from the font files provided by [Google Fonts SourceSans Pro](https://fonts.google.com/specimen/Source+Sans+Pro) 

Ensure `vim` is installed. We need its `xxd` utility.

In a terminal window, execute:  
`xxd -i font.ttf > font.c`
