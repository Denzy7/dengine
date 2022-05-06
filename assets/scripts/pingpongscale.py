import dengine.common as common
import dengine.scene as scene
import dengine.timer as timer
import math as math

def update(entity):
    current = timer.get_current()
    pingpong = math.sin(current / 1000.0)
    entity.transform.scale.z = math.fabs(pingpong) * 1.3
    entity.transform.scale.y = math.fabs(pingpong) * 0.3
