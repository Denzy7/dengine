import dengine.common as common
import dengine.scene as scene
import dengine.inpt as inpt
import dengine.timer as timer
import dengine.filesys as filesys
speed = 4.0


def start(entity):
    print("Start invoked from C to Python")
    print("files:",filesys.get_filesdir())

def update(entity):
    delta_s = timer.get_delta() / 1000.0
    if(inpt.get_key('A')):
        entity.transform.position.x -= delta_s * speed
    if(inpt.get_key('D')):
        entity.transform.position.x += delta_s * speed
    if(inpt.get_key('E')):
        entity.transform.position.y += delta_s * speed
    if(inpt.get_key('C')):
        entity.transform.position.y -= delta_s * speed
    if(inpt.get_key('W')):
        entity.transform.position.z -= delta_s * speed
    if(inpt.get_key('S')):
        entity.transform.position.z += delta_s * speed
    if(inpt.get_key('Z')):
        entity.transform.rotation.y += delta_s * speed * 30.0
    if(inpt.get_key('X')):
        entity.transform.rotation.y -= delta_s * speed * 30.0
