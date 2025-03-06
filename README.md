# Console paint (Левшуков Дмитрий Александрович [353504])

**Description:**  Console application for drawing three types of figures: triangle, circle, and rectangle, with the ability to fill, move, undo actions, and save or load the canvas to and from a file.

## **Functional requirements** 
* **Drawing** <br>
Ability to create three types of figures: triangle, circle, rectangle. Length of radius and side >=0. The validity of the figure is checked prior to its creation. The figures are drawn with the drawing symbol<br><br>
**Triangle**<br>
Drawn using the coordinates of three vertices.<br><br>
**Rectangle**<br>
Drawn using the coordinates of two vertices that lie on one diagonal.<br>
The difference in the X coordinates determines the length of one side, while the difference in the Y coordinates determines the length of the other side.<br><br>
**Circle**<br>
Drawn using the coordinates of the center and the radius.<br><br>
**Error Handling**<br>
If an invalid figure is attempted to be created, an appropriate error message is displayed to the user.

* **Filling**<br>
The figures are filled with the filling symbol. You can choose which of the drawn figures to paint.<br><br>
**Error Handling**<br>
If an invalid fill action, an appropriate error message is displayed to the user.

* **Moving**<br>
Can select any of the drawn figures to move them along the X-axis (horizontal) and Y-axis (vertical). Validity is checked before the move is applied.<br><br>
**Error Handling**<br>
If an invalid move action, an appropriate error message is displayed to the user.

* **Saving and Loading**<br>
Ability to save the current drawing to a file and load it later. Checking the correctness of the path to the file and its contents.<br><br>
**Error Handling**<br>
If an invalid save/load action, an appropriate error message is displayed to the user.

* **Undo & Redo Actions**<br>
 Ability to undo the last succesful action and redo undo action.

* **Commands**<br>
When the program starts, /help is automatically executed. When the user enters a mode, he is shown help and a list of drawn figures for executing these commands.<br><br>
"/help" - show all commands.<br>
"/set id (id = [0 - background, 1 - fill, 2 - draw]) char" - set symbol for fill, draw, background.<br>
"/move" - go to move mode (help inside mode).<br>
"/draw id x1 y1 x2 y2 x3 y3" - draw selected figure.<br>
id = 0 => rectangle (x1 y1 x2 y2) <br>
id = 1 => triangle(x1 y1 x2 y2 x3 y3) <br>
id = 2 => cirlce(x1 y1 x2). x2 is radius <br>
"/fill" - go to fill mode (help inside mode)\n"; <br>
"/erase" - go to erase mode (help inside mode)\n";<br>
"/clear" - clear palette\n";<br>
"/save path_to_file" - save to file\n";<br>
"/load path_to_file" - load from file\n";<br>
"/undo" - undo last action\n";<br>
"/redo" - redo last undo\n";<br>

## **Class diagram** 
![CD1 drawio (1)](https://github.com/user-attachments/assets/80856411-ee1f-49b7-a49c-77ced7f2b869)



