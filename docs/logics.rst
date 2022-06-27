.. |LTL| replace:: LTL
.. |LTLP| replace:: LTL+P
.. |subf| replace:: :math:`{}_f`
.. |LTLf| replace:: LTL\ |subf|
.. |LTLfP| replace:: |LTLf|\ +P
.. |LTLfMT| replace:: |LTLf|\ :sup:`MT`
   
.. :math:`\mathsf{LTL}_f^{\mathsf{MT}}`

Supported logics
=================

Here you find a brief recap of the logics supported by BLACK and their
properties. This presentation is not meant as an introduction to these logics,
but rather as a quick reference to make sure the user and BLACK are on the same
page. For more details we refer to :doc:`the Publications page <publications>`. 

Linear Temporal Logic 
----------------------

|LTL| is a propositional modal logic interpreted over infinite sequences of
states, also said *infinite words*. Each state of the sequence is labelled by a
set of *propositions* taken from a global alphabet :math:`\Sigma`. The syntax of
|LTL| extends classical propositional logic with a few *temporal operators*.
BLACK supports both *future* temporal operators, such as *tomorrow* and *until*,
and *past* temporal operators, such as *yesterday* and *since*. |LTL|, when
augmented with past operators, is often called |LTLP|.

Recently, the *finite-trace* semantics of |LTL| gained much popularity in the
field of artificial intelligence. With this semantics, the logic is often called
|LTLf|. In |LTLf|, formulas are interpreted over *finite* words. Particular
attention must be paid for the semantics of the operators to make sense. BLACK
supports both |LTL| and |LTLf|, with or without past operators (|LTLfP|).

The abstract syntax of |LTLP| and |LTLfP|, independently from the adopted
semantics, is outlined below. For the concrete input syntax accepted by BLACK,
refer to the :doc:`Input Syntax <syntax>` page. An |LTLP| (or |LTLfP|) formula
:math:`\phi` respects the following grammar:

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
   :math:`i+1 < |\overline{\sigma}|` and 
   :math:`\overline{\sigma},i+1\models \psi`
6. :math:`\overline{\sigma},i\models \mathsf{\widetilde{X}}\psi` iff 
   :math:`i+1= |\overline{\sigma}|` or
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

Note that, on infinite words, the *tomorrow* and *weak tomorrow* operators are
equivalent, so the *weak tomorrow* operator is redundant. BLACK accepts both
anyway, for uniformity, independently from the semantics chosen.

Linear Temporal Logic modulo Theories
-------------------------------------

Starting from |LTLf|, and replacing propositional letters with *first-order*
formulas over arbitrary theories, we obtain |LTLf| *Modulo Theories* (|LTLfMT|).
This is similar to how *Satisfiability Modulo Theories* extends the classical
Boolean satisfiability problem. This logic features a greatly increased
expressive power w.r.t. |LTLf|, so much indeed that the logic is in fact
*undecidable*. Nevertheless, BLACK implements a *semi-decision* procedure for
|LTLfMT|, *i.e.* a procedure that always replies **SAT** for satisfiable
formulas, and for unsatisfiable formulas may reply **UNSAT** or not terminate.

The syntax of |LTLfMT| is, as said, that of |LTLf| with first-order formulas in
place of propositions, but a few temporal operators are supported inside
first-order formulas as well. An |LTLfMT| formula :math:`\phi` respects the
following grammar:

.. math::

   \phi := {} & \psi \mid \neg\phi \mid \phi \lor \phi \mid \phi \land \phi \\
    & \phantom{\psi} \mid \mathsf{X}\phi \mid \mathsf{\widetilde{X}}\phi 
      \mid \phi\mathrel{\mathsf{U}}\phi \mid \mathsf{Y}\phi \mid \mathsf{Z}\phi 
      \mid \phi\mathrel{\mathsf{S}}\phi \\[2ex]
   \psi := {} & p(t_1,\ldots, t_n) \mid \neg\psi \mid \psi\lor\psi \mid
      \psi\land\psi \mid \exists x \psi \mid \forall x \psi \mid \\
      & \phantom{p(t_1,\ldots,t_n)} \mid \mathsf{X}\psi \mid 
      \mathsf{\widetilde{X}}\psi \mid \mathsf{Y}\psi \mid \mathsf{Z}\psi\\[2ex]
   t := {} & f(t_1,\ldots,t_m) \mid c \mid x \mid \bigcirc x \mid 
   \bigcirc\kern-1.2em\sim x

where :math:`c` is a constant, :math:`x` is a variable, :math:`p` is an
:math:`n`-ary predicate, and :math:`f` is an :math:`m`-ary function.

Intuitively, the semantics of first-order formulas is standard, and the
semantics of temporal operators follows that of |LTL|. The *term constructors*
:math:`\bigcirc x` and :math:`\bigcirc\kern-1.2em\sim x` allow the formula to
refer to the value of :math:`x` at the next state. However, :math:`\bigcirc x`
(the *strong next* term constructor), similarly to the *tomorrow* operator,
mandates the existence of such state. Instead, :math:`\bigcirc\kern-1.2em\sim x`
(the *weak next* term constructor), similarly to the *weak tomorrow* operator,
refers to such value only if the next state exists. More precisely, any atom
:math:`p(t_1,\ldots,t_n)` that contains a *strong next* term is always false at
the last state of the word, while any atom that contains a *weak next* term (but
not strong ones) is always true at the last state.

BLACK currently supports |LTLfMT| formulas over the LIA, LRA and EUF theories,
and combinations thereof.

We refer the user to :cite:`GeattiGG22` for a formal account of the semantics of
the logic.

.. note:: 
   :cite:`GeattiGG22` describes the |LTLfMT| logic 
   as was supported by BLACK in version 0.7. From v0.8, BLACK supports 
   *tomorrow* and *yesterday* (and weak versions) temporal operators inside 
   first-order formulas, and uninterpreted functions and relations are 
   *non-rigid*, *i.e.* they change over time. Moreover, BLACK supports past 
   operators, which are not covered in the paper.