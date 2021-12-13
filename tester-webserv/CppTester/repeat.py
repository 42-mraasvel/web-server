function = """TestCase testCaseStress{}() {{
	return defaultStressTest();
}}"""

for i in range(100):
	print(function.format(i))