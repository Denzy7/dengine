import dengine.inpt as inpt
import dengine.timer as timer
import dengine.logging as logging
import dengine.filesys as filesys
import dengine.scene as scene
import dengine.common as common
import time
elapsed = 0.0


def start(entity):
    entity.transform.position.x = 23.1
    print(f"Name: {entity.name}")
    print(f"pos:{entity.transform.position}")
    print(f"rot:{entity.transform.rotation}")
    print(f"scl:{entity.transform.scale}")
    logging.log("INFO::start called")
    cd = filesys.get_cachedir()
    fd = filesys.get_filesdir()
    print("cache:", cd, "files", fd)
    with open(cd+"/try.file", "w") as f:
        t = time.strftime("%d-%m-%Y, %H:%M:%S")
        f.write("hello!"+t)


def update(entity):
    global elapsed
    elapsed += timer.get_delta()
    if(elapsed > 3000.0):
        x = "mouse x " + str(inpt.get_mousex())
        logging.log(x)

        y = "mouse y " + str(inpt.get_mousey())
        logging.log(y)

        elapsed = 0.0

    if(inpt.get_key('A')):
        logging.log("WARNING::pressed a")

    if(inpt.get_key('X')):
        logging.log("ERROR::pressed x")
