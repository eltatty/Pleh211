import re
import os
import time

receipt_List = {}

# Functions
def givenAFM(afm):
    if afm in receipt_List.keys():
        for x in sorted(receipt_List[afm]):
            print("%s %.2f" % (x,receipt_List[afm][x]))


def givenPRODUCT(product):
    for x in sorted(receipt_List):
        if product in receipt_List[x]:
            print("%s %.2f" % (x,receipt_List[x][product]))



def updateDict(myafm, mydic):
    global receipt_List
    if myafm in receipt_List.keys():
        for x in mydic.keys():
            if x in receipt_List[myafm].keys():
                receipt_List[myafm][x] += mydic[x]
            else:
                receipt_List[myafm][x] = mydic[x]
    else:
        receipt_List[myafm] = mydic


def readFile(my_file):
    myfile = open("./{}".format(my_file), "r", encoding='utf-8')

    # Regex.
    pattern_lines = re.compile(r"^-*-$")
    pattern_afm = re.compile(r"(?i)\bΑΦΜ\b:{1}[ \t]+(\d{10})$")
    pattern_product = re.compile(
        r"([-a-zA-Zα-ωΑ-Ω0-9 ]*):{1}[ \t]+(\d{1,})[ \t]+(\d{1,}.\d{2})[ \t]+(\d{1,}.\d{2})$")
    pattern_sum = re.compile(r"(?i)\bΣΥΝΟΛΟ\b:{1}[ \t]+(\d{1,}.\d{2})$")

    # File read.
    match_end_lines = None
    while myfile.tell() != os.stat("./{}".format(my_file)).st_size:

        if match_end_lines == None:
            match_start_lines = re.match(pattern_lines, myfile.readline())

        if (match_start_lines != None or match_end_lines != None):
            match_afm = re.match(pattern_afm, myfile.readline())

            if match_afm != None:
                temp_afm = match_afm.group(1)
                match_product = re.match(pattern_product, myfile.readline())

                # Loop preparation
                temp_products_dict = {}
                temp_sum = 0
                match_sum = None

                while match_product != None:
                    # Check nums && Store
                    if float(match_product.group(4)) != (float(match_product.group(2)) * float(match_product.group(3))):
                        break
                    tmp_dic = {match_product.group(1).upper() : float(match_product.group(4))}
                    
                    # Warning fixed
                    if match_product.group(1).upper() in temp_products_dict.keys():
                        temp_products_dict[match_product.group(1).upper()] += float(match_product.group(4))
                    else:
                        temp_products_dict.update(tmp_dic)

                    # Procedure.
                    temp_sum += float(match_product.group(4))
                    temp_product = myfile.readline()
                    match_product = re.match(pattern_product, temp_product)
                    match_sum = re.match(pattern_sum, temp_product)


                if match_sum != None:
                    # Check nums
                    if round(temp_sum, 2) == float(match_sum.group(1)):
                        match_end_lines = re.match(
                            pattern_lines, myfile.readline())
                        if match_end_lines != None:
                            # Complete Store
                            updateDict(temp_afm, temp_products_dict)

    myfile.close()

    return


# Menu
while (True):

    print('------MENU------')
    print('Give your preference:')
    print('1. Read new input file.')
    print('2. Print statistics for a specific product.')
    print('3. Print statistics for a specific AFM.')
    print('4. Exit the program.')

    user_input = int(input())
    if user_input == 1:
        user_file = input("Give the name of the file:\n")
        try:
            readFile(user_file)
        except FileNotFoundError as e:
            print(e)
            pass
    elif user_input == 2:
        user_product = str(input("Give the name of the product:\n")).upper()
        givenPRODUCT(user_product)
    elif user_input == 3:
        user_afm = str(input("Give the afm:\n"))
        givenAFM(user_afm)
    elif user_input == 4:
        break
exit()