# generation and changes
The font sources are generated from the font files provided by [Google Fonts Roboto](https://fonts.google.com/specimen/Roboto) 

Ensure `vim` is installed. We need its `xxd` utility.

In a terminal window, execute:  
`xxd -i font.ttf > font.c`