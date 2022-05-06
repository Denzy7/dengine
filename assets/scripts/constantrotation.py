import dengine.common as common
import dengine.scene as scene
import dengine.timer as timer


speed = 4.0
def update(entity):
    delta = timer.get_delta()
    delta_s = delta / 1000.0
    entity.transform.rotation.y += delta_s * speed
    
    
