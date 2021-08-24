from PIL import Image
from PIL import ImageDraw
from PIL import ImageFont
from PIL import ImageOps
import os

def makeBharatQr(image_path):
	W, H = (288,336)
	# W, H = (144,168)

	base = Image.new("RGB",(W, H),color="white")
	# first paste the QR code
	code = Image.open("qr_codes/"+image_path)
	# code = Image.open("test/"+image_path)
	code_w, code_h = code.size
	code_x = int((W-code_w)/2)
	base.paste(code,(code_x,26))

	# then paste footer and header
	header = Image.open("css/icons/header.png")
	footer = Image.open("css/icons/footer.png")
	foot_w, foot_h = footer.size
	base.paste(header,(0,0))
	base.paste(footer,(0,H-foot_h))

	# add a border
	final = ImageOps.expand(base, border=2, fill='#FFCB05')
	final.save('finished_codes/' + image_path)	

directory = os.fsencode("qr_codes")
for file in os.listdir(directory):
    filename = os.fsdecode(file)    
    # print(filename)
    if filename.endswith(".png"): 
        print(filename)
        makeBharatQr(filename)
        continue
    else:
        continue
