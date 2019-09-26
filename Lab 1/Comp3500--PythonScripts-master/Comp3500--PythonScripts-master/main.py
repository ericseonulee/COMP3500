
# receive the file in

# for each line


#          0123456789012345678901234567890123456789012345678901234567890
#01234567891111111111222222222233333333334444444444555555555566666666
#J     20.369 : PID( 26)  JAT(    20.368) TJD(     0.958)   Mem(   0)


def create_files():
    with open("parsedOutput1.txt", 'w') as parsedFile:
        with open('output1.txt') as file1:
            for line in file1:
                if line.startswith("J") or line.startswith('*') \
                        or line.startswith("P") or line.startswith("N") \
                            or line.startswith("A"):
                    parsedFile.write(line)

    with open("parsedOutput2.txt", 'w') as parsedFile:
        with open('output2.txt') as file1:
            for line in file1:
                if line.startswith("J") or line.startswith('*') \
                        or line.startswith("P") or line.startswith("N") \
                            or line.startswith("A"):
                    parsedFile.write(line)

    with open("parsedOutput3.txt", 'w') as parsedFile:
        with open('output3.txt') as file1:
            for line in file1:
                if line.startswith("J") or line.startswith('*') \
                        or line.startswith("P") or line.startswith("N") \
                            or line.startswith("A"):
                    parsedFile.write(line)


def create_cvs1():
    unknowns = ""
    pids = ""
    jats = ""
    tjds = ""
    mems = ""
    comma = ","

    with open("csv1.txt", 'w') as csv:
        with open('parsedOutput1.txt') as file1:
            for line in file1:
                if line.startswith("*") or line.startswith("P") or line.startswith("N") or line.startswith("A"):
                    continue
                else:
                    # make line look like PID, JAT, TJD
                    unknownValue = line[1:12].replace(" ", "") + ","
                    pidValue = line[19:22].replace(" ", "") + ","
                    jatValue = line[29:39].replace(" ", "") + ","
                    tjdValue = line[45:55].replace(" ", "") + ","
                    memValue = line[63:67].replace(" ", "") + ","

                    unknowns += unknownValue
                    pids += pidValue
                    jats += jatValue
                    tjds += tjdValue
                    mems += memValue

            else:
                csv.write(unknowns)
                csv.write("\n")
                csv.write(pids)
                csv.write("\n")
                csv.write(jats)
                csv.write("\n")
                csv.write(tjds)
                csv.write("\n")
                csv.write(mems)
                csv.write("\n")


def create_cvs2():
    unknowns = ""
    pids = ""
    jats = ""
    tjds = ""
    mems = ""
    comma = ","

    with open("csv2.txt", 'w') as csv:
        with open('parsedOutput2.txt') as file1:
            for line in file1:
                if line.startswith("*") or line.startswith("P") or line.startswith("N") or line.startswith("A"):
                    continue
                else:
                    # make line look like PID, JAT, TJD
                    unknownValue = line[1:12].replace(" ", "") + ","
                    pidValue = line[19:22].replace(" ", "") + ","
                    jatValue = line[29:39].replace(" ", "") + ","
                    tjdValue = line[45:55].replace(" ", "") + ","
                    memValue = line[63:67].replace(" ", "") + ","

                    unknowns += unknownValue
                    pids += pidValue
                    jats += jatValue
                    tjds += tjdValue
                    mems += memValue

            else:
                csv.write(unknowns)
                csv.write("\n")
                csv.write(pids)
                csv.write("\n")
                csv.write(jats)
                csv.write("\n")
                csv.write(tjds)
                csv.write("\n")
                csv.write(mems)
                csv.write("\n")


def create_cvs3():
    unknowns = ""
    pids = ""
    jats = ""
    tjds = ""
    mems = ""
    comma = ","

    with open("csv3.txt", 'w') as csv:
        with open('parsedOutput3.txt') as file1:
            for line in file1:
                if line.startswith("*") or line.startswith("P") or line.startswith("N") or line.startswith("A"):
                    continue
                else:
                    # make line look like PID, JAT, TJD
                    unknownValue = line[1:12].replace(" ", "") + ","
                    pidValue = line[19:22].replace(" ", "") + ","
                    jatValue = line[29:39].replace(" ", "") + ","
                    tjdValue = line[45:55].replace(" ", "") + ","
                    memValue = line[63:67].replace(" ", "") + ","

                    unknowns += unknownValue
                    pids += pidValue
                    jats += jatValue
                    tjds += tjdValue
                    mems += memValue

            else:
                csv.write(unknowns)
                csv.write("\n")
                csv.write(pids)
                csv.write("\n")
                csv.write(jats)
                csv.write("\n")
                csv.write(tjds)
                csv.write("\n")
                csv.write(mems)
                csv.write("\n")


# Must be called first or probably something bad will happen
create_files()

create_cvs1()
create_cvs2()
create_cvs3()