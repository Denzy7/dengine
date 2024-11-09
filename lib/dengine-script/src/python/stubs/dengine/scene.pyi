from common import Vector


class Transform:
    position: Vector
    rotation: Vector
    scale: Vector


class Entity:
    name: str
    transform: Transform
