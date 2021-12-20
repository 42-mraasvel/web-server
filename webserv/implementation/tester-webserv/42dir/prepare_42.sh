# Before starting please follow the next few steps (files content can be anything and will be shown to you by the test):
# - Download the cgi_test executable on the host
# - Create a directory YoupiBanane with:
# 	-a file name youpi.bad_extension
# 	-a file name youpi.bla
# 	-a sub directory called nop
# 		-a file name youpi.bad_extension in nop
# 		-a file name other.pouic in nop
# 	-a sub directory called Yeah
# 		-a file name not_happy.bad_extension in Yeah
# press enter to continue

mkdir YoupiBanane
touch YoupiBanane/youpi.bad_extension
touch YoupiBanane/youpi.bla
mkdir YoupiBanane/nop
touch YoupiBanane/nop/youpi.bad_extension
touch YoupiBanane/nop/other.pouic
mkdir YoupiBanane/Yeah
touch YoupiBanane/Yeah/not_happy.bad_extension
