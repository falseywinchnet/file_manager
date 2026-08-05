.PHONY: test audit research

test:
	python3 -m unittest discover -s tests -v

audit:
	python3 -m compileall -q src tests experiments

research: test
	python3 experiments/e0_oracle_bootstrap/run.py
	python3 experiments/e1_gap_deletion_laws/run.py
	python3 experiments/e2_residue_addresses/run.py
	python3 experiments/e3_edit_factorization/run.py
	python3 experiments/e4_orthant_containment/run.py
	python3 experiments/e5_support_boundary_atlas/run.py
	python3 experiments/e6_cybernetic_support/run.py
