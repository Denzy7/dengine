import dengine.common as common
import dengine.scene as scene
import dengine.timer as timer
import math


def update(entity):
    current = timer.get_current()
    pingpong = math.sin(current / 1000.0)
    entity.transform.position.x = pingpong * 5.0

