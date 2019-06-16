import dolfin
import dolfin.io
import meshio

# Read the file generated by gmsh
mesh = meshio.read("input/unit_square.msh")
points, cells, cell_data, field_data = mesh.points, mesh.cells, mesh.cell_data, mesh.field_data
meshio.write("input/unit_square.xdmf", meshio.Mesh(
    points=points,
    cells={"triangle": cells["triangle"]},
    field_data=field_data))

with dolfin.io.XDMFFile(dolfin.MPI.comm_world, "input/unit_square.xdmf") as xdmf_infile:
    mesh_2d = xdmf_infile.read_mesh(dolfin.MPI.comm_world, dolfin.cpp.mesh.GhostMode.none)
    tags = xdmf_infile.read_tags()
print(tags)
# mvc = dolfin.MeshValueCollection("size_t", mesh_2d, 1)
# 
# with dolfin.io.XDMFFile(dolfin.MPI.comm_world, "input/mvc_1d.xdmf") as xdmf_infile:
#     mvc = xdmf_infile.read_mvc_size_t(mesh_2d, "name_to_read")
# 
# print("Constructing MeshFunction from MeshValueCollection")
# mf = dolfin.cpp.mesh.MeshFunctionSizet(mesh_2d, mvc, 1)

pass
