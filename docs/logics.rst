Supported logics
=================

Here you find a brief recap of the logics supported by BLACK and their
properties. This presentation is not meant as a complete introduction to these
logics, but rather as a quick reference to make sure the user and BLACK are on
the same page. For more details we refer to 
:doc:`the Publications page <publications>`. 

Linear Temporal Logic 
----------------------

:math:`\mathsf{LTL}` is a propositional modal logic interpreted over infinite
sequences of states, also said *infinite words*. Each state of the sequence is
labelled by a set of *propositions* taken from a global alphabet :math:`\Sigma`.
The syntax of :math:`\mathsf{LTL}` extends classical propositional logic with a
few *temporal operators*. BLACK supports both *future* temporal operators, such
as *tomorrow* and *until*, and *past* temporal operators, such as *yesterday*
and *since*. :math:`\mathsf{LTL}`, when augmented with past operators, is often
called :math:`\mathsf{LTL{+P}}`.

Recently, the *finite-trace* semantics of :math:`\mathsf{LTL}` gained much
popularity in the field of artificial intelligence. With this semantics, the
logic is often called :math:`\mathsf{LTL}_f`. In :math:`\mathsf{LTL}_f`,
formulas are interpreted over *finite* words. Particular attention must be paid
for the semantics of the operators to make sense. BLACK supports both
:math:`\mathsf{LTL}` and :math:`\mathsf{LTL}_f`, with or without past operators
(:math:`\mathsf{LTL}_f\mathsf{+P}`).

The abstract syntax of :math:`\mathsf{LTL}\mathsf{+P}`, independently from the
adopted semantics, is outlined below. For the concrete input syntax accepted by
BLACK, refer to the :doc:`Input Syntax <syntax>` page. An
:math:`\mathsf{LTL}\mathsf{+P}` formula :math:`\phi` respects the following
grammar:

.. math::

   \phi \equiv p & \mid \neg\phi \mid \phi \lor \phi \mid \phi \land \phi \\
    & \mid \mathsf{X}\phi \mid \mathsf{\widetilde{X}}\phi 
      \mid \phi\mathrel{\mathsf{U}}\phi \mid \mathsf{Y}\phi \mid \mathsf{Z}\phi 
      \mid \phi\mathrel{\mathsf{S}}\phi 

where :math:`p\in\Sigma`. Usual shortcuts are available, such as 
:math:`\top\equiv p \lor \neg p`, :math:`\bot\equiv\neg\top`, 
:math:`\phi_1\mathrel{\mathsf{R}}\phi_2\equiv\neg(\neg\phi_1\mathrel{\mathsf{U}}\neg\phi_2)`,
:math:`\mathsf{F}\phi\equiv \top\mathrel{\mathsf{U}}\phi`,
:math:`\mathsf{G}\phi\equiv \neg\mathsf{F}\neg \phi` (or also
:math:`\mathsf{G}\phi\equiv \bot\mathrel{\mathsf{R}}\phi`), 
:math:`\phi_1\mathrel{\mathsf{T}}\phi_2\equiv\neg(\neg\phi_1\mathrel{\mathsf{S}}\neg\phi_2)`,
:math:`\mathsf{O}\phi\equiv \top\mathrel{\mathsf{S}}\phi`, and
:math:`\mathsf{H}\phi\equiv \neg\mathsf{O}\neg \phi` (or also
:math:`\mathsf{H}\phi\equiv \bot\mathrel{\mathsf{T}}\phi`), 

A (finite or infinite) sequence 
:math:`\overline{\sigma}=\langle \sigma_0,\sigma_1,\ldots\rangle` where 
:math:`\sigma_i\in 2^\Sigma`, *satisfies* a formula :math:`\phi` at position 
:math:`i\ge0`, written :math:`\overline{\sigma},i\models\phi`, if the 
following holds:

1. :math:`\overline{\sigma},i\models p` iff :math:`p\in\sigma_i`
2. :math:`\overline{\sigma},i\models \neg\psi` iff
   :math:`\overline{\sigma},i\not\models\psi`
3. :math:`\overline{\sigma},i\models \psi_1\lor\psi_2` iff 
   :math:`\overline{\sigma},i\models \psi_1` or 
   :math:`\overline{\sigma},i\models \psi_2`
4. :math:`\overline{\sigma},i\models \psi_1\land\psi_2` iff 
   :math:`\overline{\sigma},i\models \psi_1` and 
   :math:`\overline{\sigma},i\models \psi_2`
5. :math:`\overline{\sigma},i\models \mathsf{X}\psi` iff 
   :math:`i < |\overline{\sigma}|-1` and 
   :math:`\overline{\sigma},i+1\models \psi`
6. :math:`\overline{\sigma},i\models \mathsf{\widetilde{X}}\psi` iff 
   :math:`i = |\overline{\sigma}|-1` or
   :math:`\overline{\sigma},i+1\models \psi`
7. :math:`\overline{\sigma},i\models \psi_1\mathrel{\mathsf{U}}\psi_2` iff
   there exists :math:`i \le j < |\overline{\sigma}|` such that 
   :math:`\overline{\sigma},j\models \psi_2` and 
   :math:`\overline{\sigma},k\models \psi_1` for all :math:`k` with 
   :math:`j<k <i`.
8. :math:`\overline{\sigma},i\models \mathsf{Y}\psi` iff 
   :math:`i > 0` and 
   :math:`\overline{\sigma},i-1\models \psi`
9. :math:`\overline{\sigma},i\models \mathsf{Z}\psi` iff 
   :math:`i = 0` or
   :math:`\overline{\sigma},i+1\models \psi`
10. :math:`\overline{\sigma},i\models \psi_1\mathrel{\mathsf{S}}\psi_2` iff
    there exists :math:`j \le i` such that 
    :math:`\overline{\sigma},j\models \psi_2` and 
    :math:`\overline{\sigma},k\models \psi_1` for all :math:`k` with 
    :math:`j<k\le i`.


Linear Temporal Logic modulo Theories
-------------------------------------

Starting from :math:`\mathsf{LTL}_f`, and replacing propositional letters with
*first-order* formulas over arbitrary theories, we obtain :math:`\mathsf{LTL}_f`
*modulo Theories* (:math:`\mathsf{LTL}_f^{\mathsf{MT}}`). This logic features a
greatly increased expressive power w.r.t. :math:`\mathsf{LTL}_f`, so much indeed
that the logic is in fact *undecidable*. However, BLACK implements a
*semi-decision* procedure for :math:`\mathsf{LTL}_f^{\mathsf{MT}}`, *i.e.* a
procedure that always replies :math:`\mathsf{SAT}` for satisfiable formulas, and
for unsatisfiable formulas may reply :math:`\mathsf{UNSAT}` or not terminate.

We refer the user to :cite:`GeattiGG22` for an in-depth treatment of the logic.

