# Console paint🤪🤪🤪 (Левшуков Дмитрий Александрович [353504])

**Description:**  1

**Class diagram** photo

## **Functional requirements** 
All actions put in to stack.
* **Help** - show all commands and their syntax.
* **Set** - fuction for set symbol to draw (default - '*'), fill (default - '#'), pallete (default - ' ')
* **Draw** - draw by fill symbol one of 3 type of figure by user choose:<br>
a) Triangle - draw by coordinates of 3 vertices<br>
b) Rectangle - draw by coordinates of 4 vertices or by 2 vertices on one diagonal<br>
c) Circle - draw by coordinates of center and radius<br>
Drawn figure add to the vector to future editing.
* **Fill** - dill selected figure by fill symbol.
* **Erase** - delete figure from vector and call repaint
* **Move** - move select figure and call repaint
* **Undo** - cancel last operations
* **Redo** - cancel cancelling last opreation
* **Save** - serialize figure vector to file.
* **Load** - read file, desirialize and call repaint
* **Clear** - Clear palette, delete all figures from vector.
* **Repaint** -  Clear palette and draw all figures in vector.

