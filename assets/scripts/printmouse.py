import dengine.inpt as inpt
import dengine.timer as timer
import dengine.logging as logging
import dengine.filesys as filesys
import time
elapsed = 0.0

def start():
    logging.log("INFO::start called")
    cd = filesys.get_cachedir()
    fd = filesys.get_filesdir()
    print("cache:", cd, "files", fd)
    with open(cd+"/try.file", "w") as f:
        t = time.strftime("%d-%m-%Y, %H:%M:%S")
        f.write("hello!"+t)


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
