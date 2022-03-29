import dengine.inpt as inpt

def start():
    print("Start called")

def update():
    if(inpt.get_key('A')):
        print("pressed a")
    print("mouse x", inpt.get_mousex())
    print("mouse y", inpt.get_mousey())
