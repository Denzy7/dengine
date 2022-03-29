import dengine.inpt as inpt
import dengine.timer as timer
elapsed = 0.0
def start():
    print("Start called")

def update():
    global elapsed
    elapsed += timer.get_delta()
    if(elapsed > 1000.0):
        print("mouse x", inpt.get_mousex())
        print("mouse y", inpt.get_mousey())
        elapsed = 0.0

    if(inpt.get_key('A')):
        print("pressed a")

