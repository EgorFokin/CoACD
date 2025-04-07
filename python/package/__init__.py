import numpy as np
import os

import ctypes
from ctypes import (
    Structure,
    POINTER,
    c_void_p,
    c_uint64,
    c_double,
    c_int,
    c_bool,
    c_uint,
    c_char_p,
)


_lib_files = os.listdir(os.path.dirname(os.path.abspath(__file__)))
for _file in _lib_files:
    if _file.startswith("lib_coacd"):
        _lib = ctypes.CDLL(os.path.join(os.path.dirname(os.path.abspath(__file__)), _file))



class CoACD_Mesh(ctypes.Structure):
    _fields_ = [
        ("vertices_ptr", POINTER(c_double)),
        ("vertices_count", c_uint64),
        ("triangles_ptr", POINTER(c_int)),
        ("triangles_count", c_uint64),
    ]


class CoACD_MeshArray(ctypes.Structure):
    _fields_ = [
        ("meshes_ptr", POINTER(CoACD_Mesh)),
        ("meshes_count", c_uint64),
    ]

class CoACD_Plane(Structure):
    _fields_ = [
        ("a", c_double),
        ("b", c_double),
        ("c", c_double),
        ("d", c_double),
        ("score", c_double),
    ]

class CoACD_Normalization(Structure):
    _fields_ = [
        ("mesh", CoACD_Mesh),
        ("xmin", c_double),
        ("xmax", c_double),
        ("ymin", c_double),
        ("ymax", c_double),
        ("zmin", c_double),
        ("zmax", c_double),
    ]

class CoACD_PlaneArray(ctypes.Structure):
    _fields_ = [
        ("planes_ptr", POINTER(CoACD_Plane)),
        ("planes_count", c_uint64),
    ]

class CoACD_MeshScore(Structure):
    _fields_ = [
        ("hulls_num", c_int),
        ("avg_concavity", c_double),
    ]

class CoACD_ConvexHull(Structure):
    _fields_ = [
        ("mesh", CoACD_Mesh),
        ("concavity", c_double),
    ]

_lib.CoACD_setLogLevel.argtypes = [c_char_p]
_lib.CoACD_setLogLevel.restype = None

_lib.CoACD_freeMeshArray.argtypes = [CoACD_MeshArray]
_lib.CoACD_freeMeshArray.restype = None

_lib.CoACD_run.argtypes = [
    POINTER(CoACD_Mesh),
    c_double,
    c_int,
    c_int,
    c_int,
    c_int,
    c_int,
    c_int,
    c_int,
    c_bool,
    c_bool,
    c_bool,
    c_int,
    c_bool,
    c_double,
    c_int,
    c_uint,
]
_lib.CoACD_run.restype = CoACD_MeshArray

_lib.CoACD_bestCuttingPlanes.argtypes = [
    POINTER(CoACD_Mesh),
    c_double,
    c_int,
    c_int,
    c_int,
    c_int,
    c_int,
    c_int,
    c_int,
    c_bool,
    c_bool,
    c_bool,
    c_int,
    c_bool,
    c_double,
    c_int,
    c_uint,
    c_int,
]
_lib.CoACD_bestCuttingPlanes.restype = CoACD_PlaneArray

_lib.CoACD_freePlaneArray.argtypes = [CoACD_PlaneArray]
_lib.CoACD_freePlaneArray.restype = None

_lib.CoACD_meshScore.argtypes = [
    POINTER(CoACD_Mesh),
    c_double,
    c_int,
    c_int,
    c_int,
    c_int,
    c_int,
    c_int,
    c_int,
    c_bool,
    c_bool,
    c_bool,
    c_int,
    c_bool,
    c_double,
    c_int,
    c_uint,
]
_lib.CoACD_meshScore.restype = CoACD_MeshScore

_lib.CoACD_freeMesh.argtypes = [POINTER
    (CoACD_Mesh)]
_lib.CoACD_freeMesh.restype = None


_lib.CoACD_normalize.argtypes = [
    POINTER(CoACD_Mesh),
]
_lib.CoACD_normalize.restype = CoACD_Normalization

_lib.CoACD_clip.argtypes = [
    POINTER(CoACD_Mesh),
    POINTER(CoACD_Plane),
]

_lib.CoACD_clip.restype = CoACD_MeshArray

_lib.CoACD_compute_convex_hull.argtypes = [
    POINTER(CoACD_Mesh),
]

_lib.CoACD_compute_convex_hull.restype = CoACD_ConvexHull

_lib.CoACD_merge.argtypes = [
    POINTER(CoACD_Mesh),
    CoACD_MeshArray,
    CoACD_MeshArray,
]

_lib.CoACD_merge.restype = CoACD_MeshArray


class Mesh:
    def __init__(
        self,
        vertices=np.zeros((0, 3), dtype=np.double),
        indices=np.zeros((0, 3), dtype=np.int32),
    ):
        self.vertices = np.ascontiguousarray(vertices, dtype=np.double)
        self.indices = np.ascontiguousarray(indices, dtype=np.int32)
        assert len(vertices.shape) == 2 and vertices.shape[1] == 3
        assert len(indices.shape) == 2 and indices.shape[1] == 3


def run_coacd(
    mesh: Mesh,
    threshold: float = 0.05,
    max_convex_hull: int = -1,
    preprocess_mode: str = "auto",
    preprocess_resolution: int = 30,
    resolution: int = 2000,
    mcts_nodes: int = 20,
    mcts_iterations: int = 150,
    mcts_max_depth: int = 3,
    pca: int = False,
    merge: bool = True,
    decimate: bool = False,
    max_ch_vertex: int = 256,
    extrude: bool = False,
    extrude_margin: float = 0.01,
    apx_mode: str = "ch",
    seed: int = 0,
):
    vertices = np.ascontiguousarray(mesh.vertices, dtype=np.double)
    indices = np.ascontiguousarray(mesh.indices, dtype=np.int32)
    assert len(vertices.shape) == 2 and vertices.shape[1] == 3
    assert len(indices.shape) == 2 and indices.shape[1] == 3

    mesh = CoACD_Mesh()

    mesh.vertices_ptr = ctypes.cast(
        vertices.__array_interface__["data"][0], POINTER(c_double)
    )
    mesh.vertices_count = vertices.shape[0]

    mesh.triangles_ptr = ctypes.cast(
        indices.__array_interface__["data"][0], POINTER(c_int)
    )
    mesh.triangles_count = indices.shape[0]

    if preprocess_mode == "on":
        pm = 1
    elif preprocess_mode == "off":
        pm = 2
    else:
        pm = 0

    if apx_mode == "ch":
        apx = 0
    elif apx_mode == "box":
        apx = 1

    mesh_array = _lib.CoACD_run(
        mesh,
        threshold,
        max_convex_hull,
        pm,
        preprocess_resolution,
        resolution,
        mcts_nodes,
        mcts_iterations,
        mcts_max_depth,
        pca,
        merge,
        decimate,
        max_ch_vertex,
        extrude,
        extrude_margin,
        apx,
        seed,
    )

    meshes = []
    for i in range(mesh_array.meshes_count):
        mesh = mesh_array.meshes_ptr[i]
        vertices = np.ctypeslib.as_array(
            mesh.vertices_ptr, (mesh.vertices_count, 3)
        ).copy()
        indices = np.ctypeslib.as_array(
            mesh.triangles_ptr, (mesh.triangles_count, 3)
        ).copy()
        meshes.append([vertices, indices])

    _lib.CoACD_freeMeshArray(mesh_array)
    return meshes


def set_log_level(level: str):
    level = level.encode("utf-8")
    _lib.CoACD_setLogLevel(level)

def best_cutting_planes(
        mesh: Mesh,
        threshold: float = 0.05,
        max_convex_hull: int = -1,
        preprocess_mode: str = "auto",
        preprocess_resolution: int = 30,
        resolution: int = 2000,
        mcts_nodes: int = 20,
        mcts_iterations: int = 150,
        mcts_max_depth: int = 3,
        pca: int = False,
        merge: bool = True,
        decimate: bool = False,
        max_ch_vertex: int = 256,
        extrude: bool = False,
        extrude_margin: float = 0.01,
        apx_mode: str = "ch",
        seed: int = 0,
        num_planes: int = 1,):
    vertices = np.ascontiguousarray(mesh.vertices, dtype=np.double)
    indices = np.ascontiguousarray(mesh.indices, dtype=np.int32)
    assert len(vertices.shape) == 2 and vertices.shape[1] == 3
    assert len(indices.shape) == 2 and indices.shape[1] == 3

    mesh = CoACD_Mesh()

    mesh.vertices_ptr = ctypes.cast(
        vertices.__array_interface__["data"][0], POINTER(c_double)
    )
    mesh.vertices_count = vertices.shape[0]

    mesh.triangles_ptr = ctypes.cast(
        indices.__array_interface__["data"][0], POINTER(c_int)
    )
    mesh.triangles_count = indices.shape[0]

    if preprocess_mode == "on":
        pm = 1
    elif preprocess_mode == "off":
        pm = 2
    else:
        pm = 0

    if apx_mode == "ch":
        apx = 0
    elif apx_mode == "box":
        apx = 1

    planes = _lib.CoACD_bestCuttingPlanes(
        mesh,
        threshold,
        max_convex_hull,
        pm,
        preprocess_resolution,
        resolution,
        mcts_nodes,
        mcts_iterations,
        mcts_max_depth,
        pca,
        merge,
        decimate,
        max_ch_vertex,
        extrude,
        extrude_margin,
        apx,
        seed,
        num_planes,
    )

    bestplanes = [planes.planes_ptr[i] for i in range(planes.planes_count)]
    _lib.CoACD_freePlaneArray(planes)

    return bestplanes


def mesh_score(mesh: Mesh,
        threshold: float = 0.05,
        max_convex_hull: int = -1,
        preprocess_mode: str = "auto",
        preprocess_resolution: int = 30,
        resolution: int = 2000,
        mcts_nodes: int = 20,
        mcts_iterations: int = 150,
        mcts_max_depth: int = 3,
        pca: int = False,
        merge: bool = True,
        decimate: bool = False,
        max_ch_vertex: int = 256,
        extrude: bool = False,
        extrude_margin: float = 0.01,
        apx_mode: str = "ch",
        seed: int = 0,):
    vertices = np.ascontiguousarray(mesh.vertices, dtype=np.double)
    indices = np.ascontiguousarray(mesh.indices, dtype=np.int32)
    assert len(vertices.shape) == 2 and vertices.shape[1] == 3
    assert len(indices.shape) == 2 and indices.shape[1] == 3

    mesh = CoACD_Mesh()

    mesh.vertices_ptr = ctypes.cast(
        vertices.__array_interface__["data"][0], POINTER(c_double)
    )
    mesh.vertices_count = vertices.shape[0]

    mesh.triangles_ptr = ctypes.cast(
        indices.__array_interface__["data"][0], POINTER(c_int)
    )
    mesh.triangles_count = indices.shape[0]

    if preprocess_mode == "on":
        pm = 1
    elif preprocess_mode == "off":
        pm = 2
    else:
        pm = 0

    if apx_mode == "ch":
        apx = 0
    elif apx_mode == "box":
        apx = 1

    score = _lib.CoACD_meshScore(
        mesh,
        threshold,
        max_convex_hull,
        pm,
        preprocess_resolution,
        resolution,
        mcts_nodes,
        mcts_iterations,
        mcts_max_depth,
        pca,
        merge,
        decimate,
        max_ch_vertex,
        extrude,
        extrude_margin,
        apx,
        seed,
    )

    return score
    

def normalize(mesh: Mesh, pca: bool = False):
    vertices = np.ascontiguousarray(mesh.vertices, dtype=np.double)
    indices = np.ascontiguousarray(mesh.indices, dtype=np.int32)
    assert len(vertices.shape) == 2 and vertices.shape[1] == 3
    assert len(indices.shape) == 2 and indices.shape[1] == 3

    mesh = CoACD_Mesh()

    mesh.vertices_ptr = ctypes.cast(
        vertices.__array_interface__["data"][0], POINTER(c_double)
    )
    mesh.vertices_count = vertices.shape[0]

    mesh.triangles_ptr = ctypes.cast(
        indices.__array_interface__["data"][0], POINTER(c_int)
    )
    mesh.triangles_count = indices.shape[0]

    norm = _lib.CoACD_normalize(mesh,pca)

    mesh = norm.mesh

    vertices = np.ctypeslib.as_array(
        mesh.vertices_ptr, (mesh.vertices_count, 3)
    ).copy()
    indices = np.ctypeslib.as_array(
        mesh.triangles_ptr, (mesh.triangles_count, 3)
    ).copy()

    _lib.CoACD_freeMesh(mesh)

    return (norm.xmin,norm.xmax,norm.ymin,norm.ymax,norm.zmin,norm.zmax),Mesh(vertices, indices)

def free_mesh(mesh: Mesh):
    _lib.CoACD_freeMesh(mesh)


def clip(mesh: Mesh, plane: CoACD_Plane):
    vertices = np.ascontiguousarray(mesh.vertices, dtype=np.double)
    indices = np.ascontiguousarray(mesh.indices, dtype=np.int32)
    assert len(vertices.shape) == 2 and vertices.shape[1] == 3
    assert len(indices.shape) == 2 and indices.shape[1] == 3

    mesh = CoACD_Mesh()

    mesh.vertices_ptr = ctypes.cast(
        vertices.__array_interface__["data"][0], POINTER(c_double)
    )
    mesh.vertices_count = vertices.shape[0]

    mesh.triangles_ptr = ctypes.cast(
        indices.__array_interface__["data"][0], POINTER(c_int)
    )
    mesh.triangles_count = indices.shape[0]

    mesh_array = _lib.CoACD_clip(mesh,plane)

    meshes = []
    for i in range(mesh_array.meshes_count):
        mesh = mesh_array.meshes_ptr[i]
        vertices = np.ctypeslib.as_array(
            mesh.vertices_ptr, (mesh.vertices_count, 3)
        ).copy()
        indices = np.ctypeslib.as_array(
            mesh.triangles_ptr, (mesh.triangles_count, 3)
        ).copy()
        meshes.append([vertices, indices])

    _lib.CoACD_freeMeshArray(mesh_array)
    return meshes

def compute_convex_hull(mesh: Mesh):
    vertices = np.ascontiguousarray(mesh.vertices, dtype=np.double)
    indices = np.ascontiguousarray(mesh.indices, dtype=np.int32)
    assert len(vertices.shape) == 2 and vertices.shape[1] == 3
    assert len(indices.shape) == 2 and indices.shape[1] == 3

    mesh = CoACD_Mesh()

    mesh.vertices_ptr = ctypes.cast(
        vertices.__array_interface__["data"][0], POINTER(c_double)
    )
    mesh.vertices_count = vertices.shape[0]

    mesh.triangles_ptr = ctypes.cast(
        indices.__array_interface__["data"][0], POINTER(c_int)
    )
    mesh.triangles_count = indices.shape[0]

    hull = _lib.CoACD_compute_convex_hull(mesh)

    vertices = np.ctypeslib.as_array(
        hull.mesh.vertices_ptr, (hull.mesh.vertices_count, 3)
    ).copy()
    indices = np.ctypeslib.as_array(
        hull.mesh.triangles_ptr, (hull.mesh.triangles_count, 3)
    ).copy()

    _lib.CoACD_freeMesh(hull.mesh)

    concavity = hull.concavity

    return Mesh(vertices, indices), concavity


def merge(mesh: Mesh, parts: list, convex_hulls: list):
    vertices = np.ascontiguousarray(mesh.vertices, dtype=np.double)
    indices = np.ascontiguousarray(mesh.indices, dtype=np.int32)
    assert len(vertices.shape) == 2 and vertices.shape[1] == 3
    assert len(indices.shape) == 2 and indices.shape[1] == 3

    mesh = CoACD_Mesh()

    mesh.vertices_ptr = ctypes.cast(
        vertices.__array_interface__["data"][0], POINTER(c_double)
    )
    mesh.vertices_count = vertices.shape[0]

    mesh.triangles_ptr = ctypes.cast(
        indices.__array_interface__["data"][0], POINTER(c_int)
    )
    mesh.triangles_count = indices.shape[0]

    parts_array = CoACD_MeshArray()
    parts_array.meshes_count = len(parts)
    parts_array.meshes_ptr = (CoACD_Mesh * len(parts))()

    for i, part in enumerate(parts):
        vertices = np.ascontiguousarray(part.vertices, dtype=np.double)
        indices = np.ascontiguousarray(part.indices, dtype=np.int32)
        assert len(vertices.shape) == 2 and vertices.shape[1] == 3
        assert len(indices.shape) == 2 and indices.shape[1] == 3

        part = CoACD_Mesh()

        part.vertices_ptr = ctypes.cast(
            vertices.__array_interface__["data"][0], POINTER(c_double)
        )
        part.vertices_count = vertices.shape[0]

        part.triangles_ptr = ctypes.cast(
            indices.__array_interface__["data"][0], POINTER(c_int)
        )
        part.triangles_count = indices.shape[0]

        parts_array.meshes_ptr[i] = part

    hulls = CoACD_MeshArray()
    hulls.meshes_count = len(convex_hulls)
    hulls.meshes_ptr = (CoACD_Mesh * len(convex_hulls))()

    for i, hull in enumerate(convex_hulls):
        vertices = np.ascontiguousarray(hull.vertices, dtype=np.double)
        indices = np.ascontiguousarray(hull.indices, dtype=np.int32)
        assert len(vertices.shape) == 2 and vertices.shape[1] == 3
        assert len(indices.shape) == 2 and indices.shape[1] == 3

        hull = CoACD_Mesh()

        hull.vertices_ptr = ctypes.cast(
            vertices.__array_interface__["data"][0], POINTER(c_double)
        )
        hull.vertices_count = vertices.shape[0]

        hull.triangles_ptr = ctypes.cast(
            indices.__array_interface__["data"][0], POINTER(c_int)
        )
        hull.triangles_count = indices.shape[0]

        hulls.meshes_ptr[i] = hull

    mesh_array = _lib.CoACD_merge(mesh, parts_array, hulls)

    meshes = []
    for i in range(mesh_array.meshes_count):
        mesh = mesh_array.meshes_ptr[i]
        vertices = np.ctypeslib.as_array(
            mesh.vertices_ptr, (mesh.vertices_count, 3)
        ).copy()
        indices = np.ctypeslib.as_array(
            mesh.triangles_ptr, (mesh.triangles_count, 3)
        ).copy()
        meshes.append([vertices, indices])

    _lib.CoACD_freeMeshArray(mesh_array)
    _lib.CoACD_freeMeshArray(hulls)
    _lib.CoACD_freeMeshArray(parts_array)

    return meshes

if __name__ == "__main__":
    import trimesh

    set_log_level("debug")

    mesh = trimesh.load("cow-nonormals.obj")
    mesh = Mesh(mesh.vertices, mesh.faces)
    # result = run_coacd(mesh)
    # mesh_parts = []
    # for vs, fs in result:
    #     mesh_parts.append(trimesh.Trimesh(vs, fs))

    # scene = trimesh.Scene()
    # np.random.seed(0)
    # for p in mesh_parts:
    #     p.visual.vertex_colors[:, :3] = (np.random.rand(3) * 255).astype(np.uint8)
    #     scene.add_geometry(p)
    # scene.export("decomposed.obj")

    bbox,normilized_mesh = normalize(mesh)

    # scene = trimesh.Scene()
    # scene.add_geometry(trimesh.Trimesh(normilized_mesh.vertices, normilized_mesh.indices))
    # scene.export("normalized.obj")

    # planes = best_cutting_planes(mesh,merge=False,num_planes=3)
    # for plane in planes:
    #     print(plane.a,plane.b,plane.c,plane.d, plane.score)

    # result = clip(mesh,plane)
    # mesh_parts = []
    # for vs, fs in result:
    #     mesh_parts.append(trimesh.Trimesh(vs, fs))

    # scene = trimesh.Scene()
    # np.random.seed(0)
    # for p in mesh_parts:
    #     p.visual.vertex_colors[:, :3] = (np.random.rand(3) * 255).astype(np.uint8)
    #     scene.add_geometry(p)
    # scene.export("decomposed.obj")
    

    # score = mesh_score(mesh)
    # print(score.hulls_num, score.avg_concavity)