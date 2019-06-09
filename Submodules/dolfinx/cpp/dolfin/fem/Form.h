// Copyright (C) 2007-2014 Anders Logg
//
// This file is part of DOLFIN (https://www.fenicsproject.org)
//
// SPDX-License-Identifier:    LGPL-3.0-or-later

#pragma once

#include "FormCoefficients.h"
#include "FormIntegrals.h"
#include <functional>
#include <map>
#include <memory>
#include <petscsys.h>
#include <string>
#include <vector>

// Forward declaration
struct ufc_form;

namespace dolfin
{

namespace fem
{
class CoordinateMapping;
}

namespace function
{
class FunctionSpace;
}

namespace mesh
{
class Mesh;
template <typename T>
class MeshFunction;
class Cell;
class Facet;
} // namespace mesh

namespace fem
{

/// Base class for UFC code generated by FFC for DOLFIN with option -l.
///
/// A note on the order of trial and test spaces: FEniCS numbers
/// argument spaces starting with the leading dimension of the
/// corresponding tensor (matrix). In other words, the test space is
/// numbered 0 and the trial space is numbered 1. However, in order to
/// have a notation that agrees with most existing finite element
/// literature, in particular
///
///  \f[   a = a(u, v)        \f]
///
/// the spaces are numbered from right to left
///
///  \f[   a: V_1 \times V_0 \rightarrow \mathbb{R}  \f]
///
/// This is reflected in the ordering of the spaces that should be
/// supplied to generated subclasses. In particular, when a bilinear
/// form is initialized, it should be initialized as `a(V_1, V_0) =
/// ...`, where `V_1` is the trial space and `V_0` is the test space.
/// However, when a form is initialized by a list of argument spaces
/// (the variable `function_spaces` in the constructors below), the list
/// of spaces should start with space number 0 (the test space) and then
/// space number 1 (the trial space).

class Form
{
public:
  /// Create form (shared data)
  ///
  /// @param[in] ufc_form (ufc_form)
  ///         The UFC form.
  /// @param[in] function_spaces (std::vector<_function::FunctionSpace_>)
  ///         Vector of function spaces.
  Form(const ufc_form& ufc_form,
       const std::vector<std::shared_ptr<const function::FunctionSpace>>
           function_spaces);

  /// Create form (no UFC integrals). Integrals can be attached later
  /// using FormIntegrals::set_cell_tabulate_tensor. Experimental.
  ///
  /// @param[in] function_spaces (std::vector<_function::FunctionSpace_>)
  ///         Vector of function spaces.
  Form(const std::vector<std::shared_ptr<const function::FunctionSpace>>
           function_spaces);

  /// Destructor
  virtual ~Form() = default;

  /// Return rank of form (bilinear form = 2, linear form = 1,
  /// functional = 0, etc)
  ///
  /// @return std::size_t
  ///         The rank of the form.
  std::size_t rank() const;

  /// Set coefficient with given number (shared pointer version)
  ///
  /// @param[in]  i (std::size_t)
  ///         The given number.
  /// @param[in]    coefficient (_Function_)
  ///         The coefficient.
  void set_coefficients(
      std::map<std::size_t, std::shared_ptr<const function::Function>>
          coefficients);

  /// Set coefficient with given name (shared pointer version)
  ///
  /// @param[in]    name (std::string)
  ///         The name.
  /// @param[in]    coefficient (_Function_)
  ///         The coefficient.
  void set_coefficients(
      std::map<std::string, std::shared_ptr<const function::Function>>
          coefficients);

  /// Return original coefficient position for each coefficient (0
  /// <= i < n)
  ///
  /// @return std::size_t
  ///         The position of coefficient i in original ufl form
  ///         coefficients.
  std::size_t original_coefficient_position(std::size_t i) const;

  // FIXME: remove this function. Assembler should calculate or put in
  // utils.

  /// Return the size of the element tensor, needed to create temporary
  /// space for assemblers. If the largest number of per-element dofs in
  /// function::FunctionSpace i is N_i, then for a linear form this is
  /// N_0, and for a bilinear form, N_0*N_1.
  ///
  /// @return std::size_t The maximum number of values in a local
  ///         element tensor
  std::size_t max_element_tensor_size() const;

  /// Set mesh, necessary for functionals when there are no function
  /// spaces
  ///
  /// @param[in] mesh (_mesh::Mesh_)
  ///         The mesh.
  void set_mesh(std::shared_ptr<const mesh::Mesh> mesh);

  /// Extract common mesh from form
  ///
  /// @return mesh::Mesh
  ///         Shared pointer to the mesh.
  std::shared_ptr<const mesh::Mesh> mesh() const;

  /// Return function space for given argument
  ///
  /// @param  i (std::size_t)
  ///         Index
  ///
  /// @return function::FunctionSpace
  ///         Function space shared pointer.
  std::shared_ptr<const function::FunctionSpace>
  function_space(std::size_t i) const;

  /// Return function spaces for arguments
  ///
  /// @return    std::vector<_function::FunctionSpace_>
  ///         Vector of function space shared pointers.
  std::vector<std::shared_ptr<const function::FunctionSpace>>
  function_spaces() const;

  /// Register the function for 'tabulate_tensor' for cell integral i
  void register_tabulate_tensor_cell(int i, void (*fn)(PetscScalar*,
                                                       const PetscScalar*,
                                                       const double*, int));

  /// Set cell domains
  ///
  /// @param[in]    cell_domains (_mesh::MeshFunction_ <std::size_t>)
  ///         The cell domains.
  void set_cell_domains(const mesh::MeshFunction<std::size_t>& cell_domains);

  /// Set exterior facet domains
  ///
  ///  @param[in]   exterior_facet_domains (_mesh::MeshFunction_ <std::size_t>)
  ///         The exterior facet domains.
  void set_exterior_facet_domains(
      const mesh::MeshFunction<std::size_t>& exterior_facet_domains);

  /// Set interior facet domains
  ///
  ///  @param[in]   interior_facet_domains (_mesh::MeshFunction_ <std::size_t>)
  ///         The interior facet domains.
  void set_interior_facet_domains(
      const mesh::MeshFunction<std::size_t>& interior_facet_domains);

  /// Set vertex domains
  ///
  ///  @param[in]   vertex_domains (_mesh::MeshFunction_ <std::size_t>)
  ///         The vertex domains.
  void
  set_vertex_domains(const mesh::MeshFunction<std::size_t>& vertex_domains);

  /// Access coefficients (non-const)
  FormCoefficients& coeffs() { return _coefficients; }

  /// Access coefficients (const)
  const FormCoefficients& coeffs() const { return _coefficients; }

  /// Access form integrals (const)
  const FormIntegrals& integrals() const { return _integrals; }

  /// Get coordinate_mapping (experimental)
  std::shared_ptr<const fem::CoordinateMapping> coordinate_mapping() const;

private:
  // Integrals associated with the Form
  FormIntegrals _integrals;

  // Coefficients associated with the Form
  FormCoefficients _coefficients;

  // Function spaces (one for each argument)
  std::vector<std::shared_ptr<const function::FunctionSpace>> _function_spaces;

  // The mesh (needed for functionals when we don't have any spaces)
  std::shared_ptr<const mesh::Mesh> _mesh;

  // Coordinate_mapping
  std::shared_ptr<const fem::CoordinateMapping> _coord_mapping;
};
} // namespace fem
} // namespace dolfin