TESLA Lattice Parser
=====================

example

	k = -0.5
	D: Drift, L = 5
	QF: Quad, L = 0.3, K1 = k
	QD: Quad, L = QF.L, K1 = -k
	FODO: LINE=(QF, D, QD, QD, D, QF)
	RING: LINE=(FODO, 3*FODO, -FODO) 
	
Forms from other format

	"X_Y" : Quad
	X: Quad, L = &
	  0.4, K1 = 0.01
	

