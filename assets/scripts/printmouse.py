import dengine.inpt as inpt
import dengine.timer as timer
import dengine.logging as logging

elapsed = 0.0


def start():
    logging.log("INFO::start called")


def update():
    global elapsed
    elapsed += timer.get_delta()
    if(elapsed > 1000.0):
        x = "mouse x " + str(inpt.get_mousex())
        logging.log(x)

        y = "mouse y " + str(inpt.get_mousey())
        logging.log(y)

        elapsed = 0.0

    if(inpt.get_key('A')):
        logging.log("WARNING::pressed a")

    if(inpt.get_key('X')):
        logging.log("ERROR::pressed x")
