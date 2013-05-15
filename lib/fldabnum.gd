#############################################################################
##
#W  fldabnum.gd                 GAP library                     Thomas Breuer
##
##
#Y  Copyright (C)  1996,  Lehrstuhl D für Mathematik,  RWTH Aachen,  Germany
#Y  (C) 1998 School Math and Comp. Sci., University of St Andrews, Scotland
#Y  Copyright (C) 2002 The GAP Group
##
##  This file declares operations for fields consisting of cyclotomics.
##
##  Note that we must distinguish abelian number fields and fields
##  that consist of cyclotomics.
##  (The image of the natural embedding of the rational number field
##  into a field of rational functions is of course an abelian number field
##  but its elements are not cyclotomics since this would be a property given
##  by their family.)
##


#T add rings of integers in abelian number fields!
#T (NumberRing, IsIntegralBasis, NormalBasis)

#############################################################################
##
##  Abelian Number Fields
##
##  <#GAPDoc Label="[1]{fldabnum}">
##  An <E>abelian number field</E> is a field in characteristic zero
##  that is a finite dimensional normal extension of its prime field
##  such that the Galois group is abelian.
##  In &GAP;, one implementation of abelian number fields is given by fields
##  of cyclotomic numbers (see Chapter&nbsp;<Ref Chap="Cyclotomic Numbers"/>).
##  Note that abelian number fields can also be constructed with
##  the more general <Ref Func="AlgebraicExtension"/>,
##  a discussion of advantages and disadvantages can be found
##  in&nbsp;<Ref Sect="Internally Represented Cyclotomics"/>.
##  The functions described in this chapter have been developed for fields
##  whose elements are in the filter <Ref Func="IsCyclotomic"/>,
##  they may or may not work well for abelian number fields consisting of
##  other kinds of elements.
##  <P/>
##  Throughout this chapter, <M>&QQ;_n</M> will denote the cyclotomic field
##  generated by the field <M>&QQ;</M> of rationals together with <M>n</M>-th
##  roots of unity.
##  <P/>
##  In&nbsp;<Ref Sect="Construction of Abelian Number Fields"/>,
##  constructors for abelian number fields are described,
##  <Ref Sect="Operations for Abelian Number Fields"/> introduces operations
##  for abelian number fields,
##  <Ref Sect="Integral Bases of Abelian Number Fields"/> deals with the
##  vector space structure of abelian number fields, and
##  <Ref Sect="Galois Groups of Abelian Number Fields"/> describes field
##  automorphisms of abelian number fields,
##  <!-- % section about Gaussians here? -->
##  <#/GAPDoc>
##


#############################################################################
##
#P  IsNumberField( <F> )
##
##  <#GAPDoc Label="IsNumberField">
##  <ManSection>
##  <Prop Name="IsNumberField" Arg='F'/>
##
##  <Description>
##  <Index>number field</Index>
##  returns <K>true</K> if the field <A>F</A> is a finite dimensional
##  extension of a prime field in characteristic zero,
##  and <K>false</K> otherwise.
##  </Description>
##  </ManSection>
##  <#/GAPDoc>
##
DeclareProperty( "IsNumberField", IsField );

InstallSubsetMaintenance( IsNumberField,
    IsField and IsNumberField, IsField );

InstallIsomorphismMaintenance( IsNumberField,
    IsField and IsNumberField, IsField );


#############################################################################
##
#P  IsAbelianNumberField( <F> )
##
##  <#GAPDoc Label="IsAbelianNumberField">
##  <ManSection>
##  <Prop Name="IsAbelianNumberField" Arg='F'/>
##
##  <Description>
##  <Index>abelian number field</Index>
##  returns <K>true</K> if the field <A>F</A> is a number field
##  (see&nbsp;<Ref Func="IsNumberField"/>)
##  that is a Galois extension of the prime field, with abelian Galois group
##  (see&nbsp;<Ref Oper="GaloisGroup" Label="of field"/>).
##  </Description>
##  </ManSection>
##  <#/GAPDoc>
##
DeclareProperty( "IsAbelianNumberField", IsField );

InstallTrueMethod( IsNumberField, IsAbelianNumberField );

InstallSubsetMaintenance( IsAbelianNumberField,
    IsField and IsAbelianNumberField, IsField );

InstallIsomorphismMaintenance( IsAbelianNumberField,
    IsField and IsAbelianNumberField, IsField );


#############################################################################
##
#m  Conductor( <F> )
##
##  The attribute is defined in `cyclotom.g'.
##
InstallIsomorphismMaintenance( Conductor,
    IsField and IsAbelianNumberField, IsField );


#############################################################################
##
#M  IsFieldControlledByGaloisGroup( <cycfield> )
##
##  For finite fields and abelian number fields
##  (independent of the representation of their elements),
##  we know the Galois group and have a method for `Conjugates' that does
##  not use `MinimalPolynomial'.
##
InstallTrueMethod( IsFieldControlledByGaloisGroup,
    IsField and IsAbelianNumberField );


#############################################################################
##
#P  IsCyclotomicField( <F> )
##
##  <#GAPDoc Label="IsCyclotomicField">
##  <ManSection>
##  <Prop Name="IsCyclotomicField" Arg='F'/>
##
##  <Description>
##  returns <K>true</K> if the field <A>F</A> is a <E>cyclotomic field</E>,
##  i.e., an abelian number field
##  (see&nbsp;<Ref Func="IsAbelianNumberField"/>)
##  that can be generated by roots of unity.
##  <P/>
##  <Example><![CDATA[
##  gap> IsNumberField( CF(9) ); IsAbelianNumberField( Field( [ ER(3) ] ) );
##  true
##  true
##  gap> IsNumberField( GF(2) );
##  false
##  gap> IsCyclotomicField( CF(9) );
##  true
##  gap> IsCyclotomicField( Field( [ Sqrt(-3) ] ) );
##  true
##  gap> IsCyclotomicField( Field( [ Sqrt(3) ] ) );
##  false
##  ]]></Example>
##  </Description>
##  </ManSection>
##  <#/GAPDoc>
##
DeclareProperty( "IsCyclotomicField", IsField );

InstallTrueMethod( IsAbelianNumberField, IsCyclotomicField );

InstallIsomorphismMaintenance( IsCyclotomicField,
    IsField and IsCyclotomicField, IsField );


#############################################################################
##
#A  GaloisStabilizer( <F> )
##
##  <#GAPDoc Label="GaloisStabilizer">
##  <ManSection>
##  <Attr Name="GaloisStabilizer" Arg='F'/>
##
##  <Description>
##  Let <A>F</A> be an abelian number field
##  (see&nbsp;<Ref Func="IsAbelianNumberField"/>) with conductor <M>n</M>,
##  say.
##  (This means that the <M>n</M>-th cyclotomic field is the smallest
##  cyclotomic field containing <A>F</A>,
##  see&nbsp;<Ref Func="Conductor" Label="for a cyclotomic"/>.)
##  <Ref Func="GaloisStabilizer"/> returns the set of all those integers
##  <M>k</M> in the range <M>[ 1 .. n ]</M> such that the field automorphism
##  induced by raising <M>n</M>-th roots of unity to the <M>k</M>-th power
##  acts trivially on <A>F</A>.
##  <P/>
##  <Example><![CDATA[
##  gap> r5:= Sqrt(5);
##  E(5)-E(5)^2-E(5)^3+E(5)^4
##  gap> GaloisCyc( r5, 4 ) = r5;  GaloisCyc( r5, 2 ) = r5;
##  true
##  false
##  gap> GaloisStabilizer( Field( [ r5 ] ) );
##  [ 1, 4 ]
##  ]]></Example>
##  </Description>
##  </ManSection>
##  <#/GAPDoc>
##
DeclareAttribute( "GaloisStabilizer", IsAbelianNumberField );

InstallIsomorphismMaintenance( GaloisStabilizer,
    IsField and IsAbelianNumberField, IsField );


#############################################################################
##
#V  Rationals . . . . . . . . . . . . . . . . . . . . . .  field of rationals
#P  IsRationals( <obj> )
##
##  <#GAPDoc Label="Rationals">
##  <ManSection>
##  <Var Name="Rationals"/>
##  <Prop Name="IsRationals" Arg='obj'/>
##
##  <Description>
##  <Ref Var="Rationals"/> is the field <M>&QQ;</M> of rational integers,
##  as a set of cyclotomic numbers,
##  see Chapter&nbsp;<Ref Chap="Cyclotomic Numbers"/> for basic operations,
##  Functions for the field <Ref Var="Rationals"/> can be found in the
##  chapters&nbsp;<Ref Chap="Fields and Division Rings"/>
##  and&nbsp;<Ref Chap="Abelian Number Fields"/>.
##  <P/>
##  <Ref Prop="IsRationals"/> returns <K>true</K> for a prime field that
##  consists of cyclotomic numbers
##  &ndash;for example the &GAP; object <Ref Var="Rationals"/>&ndash;
##  and <K>false</K> for all other &GAP; objects.
##  <P/>
##  <Example><![CDATA[
##  gap> Size( Rationals ); 2/3 in Rationals;
##  infinity
##  true
##  ]]></Example>
##  </Description>
##  </ManSection>
##  <#/GAPDoc>
##
DeclareGlobalVariable( "Rationals", "field of rationals" );

DeclareSynonym( "IsRationals",
    IsCyclotomicCollection and IsField and IsPrimeField );

InstallTrueMethod( IsCyclotomicField, IsRationals );


#############################################################################
##
#V  GaussianRationals . . . . . . . . . . . . . . field of Gaussian rationals
#C  IsGaussianRationals( <obj> )
##
##  <#GAPDoc Label="GaussianRationals">
##  <ManSection>
##  <Var Name="GaussianRationals"/>
##  <Filt Name="IsGaussianRationals" Arg='obj' Type='Category'/>
##
##  <Description>
##  <Ref Func="GaussianRationals"/> is the field
##  <M>&QQ;_4 = &QQ;(\sqrt{{-1}})</M> of Gaussian rationals,
##  as a set of cyclotomic numbers,
##  see Chapter&nbsp;<Ref Chap="Cyclotomic Numbers"/> for basic operations.
##  This field can also be obtained as <C>CF(4)</C>
##  (see <Ref Func="CyclotomicField" Label="for (subfield and) conductor"/>).
##  <P/>
##  The filter <Ref Func="IsGaussianRationals"/> returns <K>true</K> for the
##  &GAP; object <Ref Var="GaussianRationals"/>,
##  and <K>false</K> for all other &GAP; objects.
##  <P/>
##  (For details about the field of rationals,
##  see Chapter&nbsp;<Ref Func="Rationals"/>.)
##  <P/>
##  <Example><![CDATA[
##  gap> CF(4) = GaussianRationals;
##  true
##  gap> Sqrt(-1) in GaussianRationals;
##  true
##  ]]></Example>
##  </Description>
##  </ManSection>
##  <#/GAPDoc>
##
DeclareGlobalVariable( "GaussianRationals",
    "field of Gaussian rationals (identical with CF(4))" );

DeclareCategory( "IsGaussianRationals", IsCyclotomicCollection and IsField );
#T better?


#############################################################################
##
#V  CYCLOTOMIC_FIELDS
##
##  <ManSection>
##  <Var Name="CYCLOTOMIC_FIELDS"/>
##
##  <Description>
##  At position <A>n</A>, the <A>n</A>-th cyclotomic field is stored.
##  </Description>
##  </ManSection>
##
DeclareGlobalVariable( "CYCLOTOMIC_FIELDS",
    "list, CYCLOTOMIC_FIELDS[n] = CF(n) if bound" );

InstallFlushableValue( CYCLOTOMIC_FIELDS, [ ] );
ShareObj(CYCLOTOMIC_FIELDS);

#############################################################################
##
#F  CyclotomicField( [<subfield>, ]<n> ) . create the <n>-th cyclotomic field
#F  CyclotomicField( [<subfield>, ]<gens> )
#F  CF( [<subfield>, ]<n> )
#F  CF( [<subfield>, ]<gens> )
##
##  <#GAPDoc Label="CyclotomicField">
##  <ManSection>
##  <Func Name="CyclotomicField" Arg='[subfield, ]n'
##   Label="for (subfield and) conductor"/>
##  <Func Name="CyclotomicField" Arg='[subfield, ]gens'
##   Label="for (subfield and) generators"/>
##  <Func Name="CF" Arg='[subfield, ]n'
##   Label="for (subfield and) conductor"/>
##  <Func Name="CF" Arg='[subfield, ]gens'
##   Label="for (subfield and) generators"/>
##
##  <Description>
##  The first version creates the <A>n</A>-th cyclotomic field <M>&QQ;_n</M>.
##  The second version creates the smallest cyclotomic field containing the
##  elements in the list <A>gens</A>.
##  In both cases the field can be generated as an extension of a designated
##  subfield <A>subfield</A>
##  (cf.&nbsp;<Ref Sect="Integral Bases of Abelian Number Fields"/>).
##  <P/>
##  <Ref Func="CyclotomicField" Label="for (subfield and) conductor"/> can be
##  abbreviated to <Ref Func="CF" Label="for (subfield and) conductor"/>,
##  this form is used also when &GAP; prints cyclotomic fields.
##  <P/>
##  Fields constructed with the one argument version of
##  <Ref Func="CF" Label="for (subfield and) conductor"/>
##  are stored in the global list <C>CYCLOTOMIC_FIELDS</C>,
##  so repeated calls of
##  <Ref Func="CF" Label="for (subfield and) conductor"/> just fetch these
##  field objects after they have been created once.
##  <!--  The cache can be flushed by ...-->
##  <P/>
##  <Example><![CDATA[
##  gap> CyclotomicField( 5 );  CyclotomicField( [ Sqrt(3) ] );
##  CF(5)
##  CF(12)
##  gap> CF( CF(3), 12 );  CF( CF(4), [ Sqrt(7) ] );
##  AsField( CF(3), CF(12) )
##  AsField( GaussianRationals, CF(28) )
##  ]]></Example>
##  </Description>
##  </ManSection>
##  <#/GAPDoc>
##
DeclareGlobalFunction( "CyclotomicField" );

DeclareSynonym( "CF", CyclotomicField );


#############################################################################
##
#V  ABELIAN_NUMBER_FIELDS
##
##  <ManSection>
##  <Var Name="ABELIAN_NUMBER_FIELDS"/>
##
##  <Description>
##  At position <A>n</A>, those fields with conductor <A>n</A> are stored
##  that are not cyclotomic fields.
##  The list for cyclotomic fields is <C>CYCLOTOMIC_FIELDS</C>.
##  </Description>
##  </ManSection>
##
DeclareGlobalVariable( "ABELIAN_NUMBER_FIELDS",
    "list of lists, at position [1][n] stabilizers, at [2][n] the fields" );
InstallFlushableValue( ABELIAN_NUMBER_FIELDS, [ [], [] ] );
ShareObj(ABELIAN_NUMBER_FIELDS);


#############################################################################
##
#F  AbelianNumberField( <n>, <stab> ) . . . .  create an abelian number field
##
##  <#GAPDoc Label="AbelianNumberField">
##  <ManSection>
##  <Func Name="AbelianNumberField" Arg='n, stab'/>
##  <Func Name="NF" Arg='n, stab'/>
##
##  <Description>
##  For a positive integer <A>n</A> and a list <A>stab</A> of prime residues
##  modulo <A>n</A>,
##  <Ref Func="AbelianNumberField"/> returns the fixed field of the group
##  described by <A>stab</A> (cf.&nbsp;<Ref Func="GaloisStabilizer"/>),
##  in the <A>n</A>-th cyclotomic field.
##  <Ref Func="AbelianNumberField"/> is mainly thought for internal use
##  and for printing fields in a standard way;
##  <Ref Func="Field" Label="for several generators"/>
##  (cf.&nbsp;also&nbsp;<Ref Sect="Operations for Abelian Number Fields"/>)
##  is probably more suitable if one knows generators of the field in
##  question.
##  <P/>
##  <Ref Func="AbelianNumberField"/> can be abbreviated to <Ref Func="NF"/>,
##  this form is used also when &GAP; prints abelian number fields.
##  <P/>
##  Fields constructed with <Ref Func="NF"/> are stored in the global list
##  <C>ABELIAN_NUMBER_FIELDS</C>,
##  so repeated calls of <Ref Func="NF"/> just fetch these field objects
##  after they have been created once.
##  <!--  The cache can be flushed by ...-->
##  <P/>
##  <Example><![CDATA[
##  gap> NF( 7, [ 1 ] );
##  CF(7)
##  gap> f:= NF( 7, [ 1, 2 ] );  Sqrt(-7); Sqrt(-7) in f;
##  NF(7,[ 1, 2, 4 ])
##  E(7)+E(7)^2-E(7)^3+E(7)^4-E(7)^5-E(7)^6
##  true
##  ]]></Example>
##  </Description>
##  </ManSection>
##  <#/GAPDoc>
##
DeclareGlobalFunction( "AbelianNumberField" );

DeclareSynonym( "NF", AbelianNumberField );
DeclareSynonym( "NumberField", AbelianNumberField );


#############################################################################
##
##  <#GAPDoc Label="[2]{fldabnum}">
##  Each abelian number field is naturally a vector space over <M>&QQ;</M>.
##  Moreover, if the abelian number field <M>F</M> contains the <M>n</M>-th
##  cyclotomic field <M>&QQ;_n</M> then <M>F</M> is a vector space over
##  <M>&QQ;_n</M>.
##  In &GAP;, each field object represents a vector space object over a
##  certain subfield <M>S</M>, which depends on the way <M>F</M> was
##  constructed.
##  The subfield <M>S</M> can be accessed as the value of the attribute
##  <Ref Func="LeftActingDomain"/>.
##  <P/>
##  The return values of <Ref Func="NF"/> and of the one argument
##  versions of <Ref Func="CF" Label="for (subfield and) conductor"/>
##  represent vector spaces over <M>&QQ;</M>,
##  and the return values of the two argument version of
##  <Ref Func="CF" Label="for (subfield and) conductor"/>
##  represent vector spaces over the field that is given as the first
##  argument.
##  For an abelian number field <A>F</A> and a subfield <A>S</A> of <A>F</A>,
##  a &GAP; object representing <A>F</A> as a vector space over <A>S</A> can
##  be constructed using <Ref Func="AsField"/>.
##  <P/>
##  <Index Subkey="CanonicalBasis">cyclotomic fields</Index>
##  Let <A>F</A> be the cyclotomic field <M>&QQ;_n</M>,
##  represented as a vector space over the subfield <A>S</A>.
##  If <A>S</A> is the cyclotomic field <M>&QQ;_m</M>,
##  with <M>m</M> a divisor of <M>n</M>,
##  then <C>CanonicalBasis( <A>F</A> )</C> returns the Zumbroich basis of
##  <A>F</A> relative to <A>S</A>,
##  which consists of the roots of unity <C>E(<A>n</A>)</C>^<A>i</A>
##  where <A>i</A> is an element of the list
##  <C>ZumbroichBase( <A>n</A>, <A>m</A> )</C>
##  (see&nbsp;<Ref Func="ZumbroichBase"/>).
##  If <A>S</A> is an abelian number field that is not a cyclotomic field
##  then <C>CanonicalBasis( <A>F</A> )</C> returns a normal <A>S</A>-basis
##  of <A>F</A>, i.e., a basis that is closed under the field automorphisms
##  of <A>F</A>.
##  <P/>
##  <Index Subkey="CanonicalBasis">abelian number fields</Index>
##  Let <A>F</A> be the abelian number field
##  <C>NF( <A>n</A>, <A>stab</A> )</C>, with conductor
##  <A>n</A>, that is itself not a cyclotomic field,
##  represented as a vector space over the subfield <A>S</A>.
##  If <A>S</A> is the cyclotomic field <M>&QQ;_m</M>,
##  with <M>m</M> a divisor of <M>n</M>,
##  then <C>CanonicalBasis( <A>F</A> )</C> returns the Lenstra basis of
##  <A>F</A> relative to <A>S</A> that consists of the sums of roots of unity
##  described by
##  <C>LenstraBase( <A>n</A>, <A>stab</A>, <A>stab</A>, <A>m</A> )</C>
##  (see&nbsp;<Ref Func="LenstraBase"/>).
##  If <A>S</A> is an abelian number field that is not a cyclotomic field
##  then <C>CanonicalBasis( <A>F</A> )</C> returns a normal <A>S</A>-basis
##  of <A>F</A>.
##  <P/>
##  <Example><![CDATA[
##  gap> f:= CF(8);;   # a cycl. field over the rationals
##  gap> b:= CanonicalBasis( f );;  BasisVectors( b );
##  [ 1, E(8), E(4), E(8)^3 ]
##  gap> Coefficients( b, Sqrt(-2) );
##  [ 0, 1, 0, 1 ]
##  gap> f:= AsField( CF(4), CF(8) );;  # a cycl. field over a cycl. field
##  gap> b:= CanonicalBasis( f );;  BasisVectors( b );
##  [ 1, E(8) ]
##  gap> Coefficients( b, Sqrt(-2) );
##  [ 0, 1+E(4) ]
##  gap> f:= AsField( Field( [ Sqrt(-2) ] ), CF(8) );;
##  gap> # a cycl. field over a non-cycl. field
##  gap> b:= CanonicalBasis( f );;  BasisVectors( b );
##  [ 1/2+1/2*E(8)-1/2*E(8)^2-1/2*E(8)^3, 
##    1/2-1/2*E(8)+1/2*E(8)^2+1/2*E(8)^3 ]
##  gap> Coefficients( b, Sqrt(-2) );
##  [ E(8)+E(8)^3, E(8)+E(8)^3 ]
##  gap> f:= Field( [ Sqrt(-2) ] );   # a non-cycl. field over the rationals
##  NF(8,[ 1, 3 ])
##  gap> b:= CanonicalBasis( f );;  BasisVectors( b );
##  [ 1, E(8)+E(8)^3 ]
##  gap> Coefficients( b, Sqrt(-2) );
##  [ 0, 1 ]
##  ]]></Example>
##  <#/GAPDoc>
##


#############################################################################
##
#F  ZumbroichBase( <n>, <m> )
##
##  <#GAPDoc Label="ZumbroichBase">
##  <ManSection>
##  <Func Name="ZumbroichBase" Arg='n, m'/>
##
##  <Description>
##  Let <A>n</A> and <A>m</A> be positive integers,
##  such that <A>m</A> divides <A>n</A>.
##  <Ref Func="ZumbroichBase"/> returns the set of exponents <M>i</M>
##  for which <C>E(<A>n</A>)^</C><M>i</M> belongs to the (generalized)
##  Zumbroich basis of the cyclotomic field <M>&QQ;_n</M>,
##  viewed as a vector space over <M>&QQ;_m</M>.
##  <P/>
##  This basis is defined as follows.
##  Let <M>P</M> denote the set of prime divisors of <A>n</A>,
##  <M><A>n</A> = \prod_{{p \in P}} p^{{\nu_p}}</M>, and
##  <M><A>m</A> = \prod_{{p \in P}} p^{{\mu_p}}</M>
##  with <M>\mu_p \leq \nu_p</M>.
##  Let <M>e_l =</M> <C>E</C><M>(l)</M> for any positive integer <M>l</M>, 
##  and
##  <M>\{ e_{{n_1}}^j \}_{{j \in J}} \otimes \{ e_{{n_2}}^k \}_{{k \in K}} =
##  \{ e_{{n_1}}^j \cdot e_{{n_2}}^k \}_{{j \in J, k \in K}}</M>.
##  <P/>
##  Then the basis is
##  <Display Mode="M">
##  B_{{n,m}} = \bigotimes_{{p \in P}}
##    \bigotimes_{{k = \mu_p}}^{{\nu_p-1}}
##       \{ e_{{p^{{k+1}}}}^j \}_{{j \in J_{{k,p}}}}
##  </Display>
##  where <M>J_{{k,p}} =</M>
##  <Table Align="lcl">
##  <Row>
##     <Item><M>\{ 0 \}</M></Item>
##     <Item>;</Item>
##     <Item><M>k = 0, p = 2</M></Item>
##  </Row>
##  <Row>
##     <Item><M>\{ 0, 1 \}</M></Item>
##     <Item>;</Item>
##     <Item><M>k &gt; 0, p = 2</M></Item>
##  </Row>
##  <Row>
##     <Item><M>\{ 1, \ldots, p-1 \}</M></Item>
##     <Item>;</Item>
##     <Item><M>k = 0, p \neq 2</M></Item>
##  </Row>
##  <Row>
##     <Item><M>\{ -(p-1)/2, \ldots, (p-1)/2 \}</M></Item>
##     <Item>;</Item>
##     <Item><M>k &gt; 0, p \neq 2</M></Item>
##  </Row>
##  </Table>
##  <P/>
##  <M>B_{{n,1}}</M> is equal to the basis of <M>&QQ;_n</M>
##  over the rationals which is introduced in&nbsp;<Cite Key="Zum89"/>.
##  Also the conversion of arbitrary sums of roots of unity into its
##  basis representation, and the reduction to the minimal cyclotomic field
##  are described in this thesis.
##  (Note that the notation here is slightly different from that there.)
##  <P/>
##  <M>B_{{n,m}}</M> consists of roots of unity, it is an integral basis
##  (that is, exactly the integral elements in <M>&QQ;_n</M> have integral
##  coefficients w.r.t.&nbsp;<M>B_{{n,m}}</M>,
##  cf.&nbsp;<Ref Func="IsIntegralCyclotomic"/>),
##  it is a normal basis for squarefree <M>n</M>
##  and closed under complex conjugation for odd <M>n</M>.
##  <P/>
##  <E>Note:</E>
##  For <M><A>n</A> \equiv 2 \pmod 4</M>, we have
##  <C>ZumbroichBase(<A>n</A>, 1) = 2 * ZumbroichBase(<A>n</A>/2, 1)</C> and
##  <C>List( ZumbroichBase(<A>n</A>, 1), x -> E(<A>n</A>)^x ) =
##   List( ZumbroichBase(<A>n</A>/2, 1), x -> E(<A>n</A>/2)^x )</C>.
##  <P/>
##  <Example><![CDATA[
##  gap> ZumbroichBase( 15, 1 ); ZumbroichBase( 12, 3 );
##  [ 1, 2, 4, 7, 8, 11, 13, 14 ]
##  [ 0, 3 ]
##  gap> ZumbroichBase( 10, 2 ); ZumbroichBase( 32, 4 );
##  [ 2, 4, 6, 8 ]
##  [ 0, 1, 2, 3, 4, 5, 6, 7 ]
##  ]]></Example>
##  </Description>
##  </ManSection>
##  <#/GAPDoc>
##
DeclareGlobalFunction( "ZumbroichBase" );


#############################################################################
##
#F  LenstraBase( <n>, <stabilizer>, <super>, <m> )
##
##  <#GAPDoc Label="LenstraBase">
##  <ManSection>
##  <Func Name="LenstraBase" Arg='n, stabilizer, super, m'/>
##
##  <Description>
##  Let <A>n</A> and <A>m</A> be positive integers
##  such that <A>m</A> divides <A>n</A>,
##  <A>stabilizer</A> be a list of prime residues modulo <A>n</A>,
##  which describes a subfield of the <A>n</A>-th cyclotomic field
##  (see&nbsp;<Ref Func="GaloisStabilizer"/>),
##  and <A>super</A> be a list representing a supergroup of the group given by
##  <A>stabilizer</A>.
##  <P/>
##  <Ref Func="LenstraBase"/> returns a list <M>[ b_1, b_2, \ldots, b_k ]</M>
##  of lists, each <M>b_i</M> consisting of integers such that the elements
##  <M>\sum_{{j \in b_i}} </M><C>E(n)</C><M>^j</M> form a basis of the
##  abelian number field <C>NF( <A>n</A>, <A>stabilizer</A> )</C>,
##  as a vector space over the <A>m</A>-th cyclotomic field
##  (see&nbsp;<Ref Func="AbelianNumberField"/>).
##  <P/>
##  This basis is an integral basis,
##  that is, exactly the integral elements in
##  <C>NF( <A>n</A>, <A>stabilizer</A> )</C>
##  have integral coefficients.
##  (For details about this basis, see&nbsp;<Cite Key="Bre97"/>.)
##  <P/>
##  If possible then the result is chosen such that the group described by
##  <A>super</A> acts on it,
##  consistently with the action of <A>stabilizer</A>, i.e.,
##  each orbit of <A>super</A> is a union of orbits of <A>stabilizer</A>.
##  (A usual case is <A>super</A><C> = </C><A>stabilizer</A>,
##  so there is no additional condition.
##  <P/>
##  <E>Note:</E>
##  The <M>b_i</M> are in general not sets,
##  since for <C><A>stabilizer</A> = <A>super</A></C>,
##  the first entry is always an element of
##  <C>ZumbroichBase( <A>n</A>, <A>m</A> )</C>;
##  this property is used by <Ref Func="NF"/> and <Ref Func="Coefficients"/>
##  (see&nbsp;<Ref Sect="Integral Bases of Abelian Number Fields"/>).
##  <P/>
##  <A>stabilizer</A> must not contain the stabilizer of a proper
##  cyclotomic subfield of the <A>n</A>-th cyclotomic field, i.e.,
##  the result must describe a basis for a field with conductor <A>n</A>.
##  <P/>
##  <Example><![CDATA[
##  gap> LenstraBase( 24, [ 1, 19 ], [ 1, 19 ], 1 );
##  [ [ 1, 19 ], [ 8 ], [ 11, 17 ], [ 16 ] ]
##  gap> LenstraBase( 24, [ 1, 19 ], [ 1, 5, 19, 23 ], 1 );
##  [ [ 1, 19 ], [ 5, 23 ], [ 8 ], [ 16 ] ]
##  gap> LenstraBase( 15, [ 1, 4 ], PrimeResidues( 15 ), 1 );
##  [ [ 1, 4 ], [ 2, 8 ], [ 7, 13 ], [ 11, 14 ] ]
##  ]]></Example>
##  <P/>
##  The first two results describe two bases of the field
##  <M>&QQ;_3(\sqrt{{6}})</M>,
##  the third result describes a normal basis of <M>&QQ;_3(\sqrt{{5}})</M>.
##  </Description>
##  </ManSection>
##  <#/GAPDoc>
##
DeclareGlobalFunction( "LenstraBase" );


#############################################################################
##
#V  Cyclotomics . . . . . . . . . . . . . . . . . . domain of all cyclotomics
##
##  <#GAPDoc Label="Cyclotomics">
##  <ManSection>
##  <Var Name="Cyclotomics"/>
##
##  <Description>
##  is the domain of all cyclotomics.
##  <P/>
##  <Example><![CDATA[
##  gap> E(9) in Cyclotomics; 37 in Cyclotomics; true in Cyclotomics;
##  true
##  true
##  false
##  ]]></Example>
##  <P/>
##  As the cyclotomics are field elements, the usual arithmetic operators
##  <C>+</C>, <C>-</C>, <C>*</C> and <C>/</C> (and <C>^</C> to take powers by
##  integers) are applicable.
##  Note that <C>^</C> does <E>not</E> denote the conjugation of group
##  elements, so it is <E>not</E> possible to explicitly construct groups of
##  cyclotomics.
##  (However, it is possible to compute the inverse and the multiplicative
##  order of a nonzero cyclotomic.)
##  Also, taking the <M>k</M>-th power of a root of unity <M>z</M> defines a
##  Galois automorphism if and only if <M>k</M> is coprime to the conductor
##  (see <Ref Func="Conductor" Label="for a cyclotomic"/>) of <M>z</M>.
##  <P/>
##  <Example><![CDATA[
##  gap> E(5) + E(3); (E(5) + E(5)^4) ^ 2; E(5) / E(3); E(5) * E(3);
##  -E(15)^2-2*E(15)^8-E(15)^11-E(15)^13-E(15)^14
##  -2*E(5)-E(5)^2-E(5)^3-2*E(5)^4
##  E(15)^13
##  E(15)^8
##  gap> Order( E(5) ); Order( 1+E(5) );
##  5
##  infinity
##  ]]></Example>
##  </Description>
##  </ManSection>
##  <#/GAPDoc>
##
DeclareGlobalVariable( "Cyclotomics", "domain of all cyclotomics" );


#############################################################################
##
#F  ANFAutomorphism( <F>, <k> )  . .  automorphism of an abelian number field
##
##  <#GAPDoc Label="ANFAutomorphism">
##  <ManSection>
##  <Func Name="ANFAutomorphism" Arg='F, k'/>
##
##  <Description>
##  Let <A>F</A> be an abelian number field and <A>k</A> be an integer
##  that is coprime to the conductor
##  (see <Ref Func="Conductor" Label="for a collection of cyclotomics"/>)
##  of <A>F</A>.
##  Then <Ref Func="ANFAutomorphism"/> returns the automorphism of <A>F</A>
##  that is defined as the linear extension of the map that raises each root
##  of unity in <A>F</A> to its <A>k</A>-th power.
##  <P/>
##  <Example><![CDATA[
##  gap> f:= CF(25);
##  CF(25)
##  gap> alpha:= ANFAutomorphism( f, 2 );
##  ANFAutomorphism( CF(25), 2 )
##  gap> alpha^2;
##  ANFAutomorphism( CF(25), 4 )
##  gap> Order( alpha );
##  20
##  gap> E(5)^alpha;
##  E(5)^2
##  ]]></Example>
##  </Description>
##  </ManSection>
##  <#/GAPDoc>
##
DeclareGlobalFunction( "ANFAutomorphism" );


#############################################################################
##
#A  ExponentOfPowering( <map> )
##
##  <ManSection>
##  <Attr Name="ExponentOfPowering" Arg='map'/>
##
##  <Description>
##  For a mapping <A>map</A> that raises each element of its preimage
##  to the same positive power, <Ref Attr="ExponentOfPowering"/> returns
##  the smallest positive number <M>n</M> with this property.
##  <P/>
##  Examples of such mappings are Frobenius automorphisms
##  (see&nbsp;<Ref Sect="FrobeniusAutomorphism"/>).
##  <P/>
##  The action of a Galois automorphism of an abelian number field is given
##  by the <M>&QQ;</M>-linear extension of raising each root of unity to
##  the same power <M>n</M>, see&nbsp;<Ref Func="ANFAutomorphism"/>.
##  For such a field automorphism, <Ref Attr="ExponentOfPowering"/> returns
##  <M>n</M>.
##  </Description>
##  </ManSection>
##
DeclareAttribute( "ExponentOfPowering", IsMapping );


#############################################################################
##
#E

