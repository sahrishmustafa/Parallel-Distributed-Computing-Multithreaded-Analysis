#!/usr/bin/env python
# Filename: extract.py
# to extract available file list for certain language pairs or triples and so on
# python extract.py lang1 lang2 lang3

import sys, string, os, re, codecs
from xml.sax import saxutils

outputDir="./text"
xmlroot="./xml"


#'''
langCode={'Chinese':"zh", 
          'Arabic':"ar", 
          'French':"fr", 
          'Spanish':"es",
          'Russian':"ru", 
          'English': "en",
          'Other':"de",
          'German':"de"
          }

langName={"zh":'Chinese', 
          "ar":'Arabic', 
          "fr":'French', 
          "es":'Spanish',
          "ru":'Russian', 
          "en":'English',
          "de":'Other'
          }
#'''


help_message = '''Usage: python extract.py lang1 lang2 lang3....
Languages: Arabic(ar), Chinese(zh), English(en), French(fr), German(de), Spanish(es), Russian(ru)'''


def warning(message):
    print (sys.stderr, message)
    sys.exit()


stag=re.compile(r"^[ \t]*<s n=\"\d+\">[ \t]*(.*)[ \t]*</s>[ \t]*$")
tag=re.compile(r"<[^>]*>")

def extract_text(filename, lang="en"):
    f=open(filename, 'r')

#    tok_proc=tokenizer
    text=""
    for line in f:
        line=line.strip()

        if line.find("<s")>=0:
            sentence=stag.search(line).groups()[0]+"\n"

            if not len(sentence.strip()):
                continue

            tagged=re.findall(r"<[^>]*>[^<]*</[^>]*>", sentence)
            tagDict={}

            if len(tagged):
                for i in range(len(tagged)):
                    line=tagged[i]
                    tagType=line.lstrip("<").split(">")[0].upper()+" - "+str(i)
                    tagDict[tagType]=line.lstrip("<").split(">")[1].split("<")[0]
                    sentence=sentence.replace(line,tagType,1)

            tok_sent=sentence.strip()

            if len(tagged):
                for t, l in tagDict.items():
                    tok_sent=tok_sent.replace(t, l)

            text+=tok_sent.strip()+"\n"
        if line.find("</p")>=0:
            text+="\n"
    f.close()
    return saxutils.unescape(text.strip(), {'&quot;':'"', "&apos;":"'"})

if len(sys.argv)<2:
    warning(help_message)    

if sys.argv[1].startswith("-"):
    if sys.argv[1] == "-t":
        outputDir+="-test/"
        xmlroot+="-test/"
        langs=sys.argv[2:]
    else:
        warning("ERROR: Unsupported option.\n"+help_message)
else:
    outputDir+="/"
    xmlroot+="/"
    langs=sys.argv[1:]


# if len(langs)==0:
#     warning(help_message)
# elif len(langs)<2:
#     warning("Indicate at least two languages!\n"+help_message)




for i, l in enumerate(langs):
    if l in langCode:
        langs[i] = langCode[l]  # Convert language name to code if applicable
    elif l not in langName.values():  # Ensure the language is valid
        warning("%s files not found.\n" % l + help_message)

count=0



textdir=outputDir+"-".join(langs)

if not os.path.exists(outputDir):
    os.mkdir(outputDir)
if not os.path.exists(textdir):
    os.mkdir(textdir)




for y in range(2000,2100):

    year=str(y)

    if not os.path.exists(xmlroot+"en/%s"%year):
        continue

    targetdir=textdir+"/"+year+'/'

    if not os.path.exists(targetdir):
        os.mkdir(targetdir)

    for filename in os.listdir(xmlroot+"%s/%s/"%(langs[0], year)):

        docid=filename.split("/")[-1].rsplit('-', 1)[0]
        parallel=True

        for lang in langs[1:]:
            altFile=xmlroot+lang+"/"+year+'/'+docid+"-"+lang+".xml"

            if not os.path.exists(altFile):
                parallel=False
                continue

        if not parallel:
            continue
        
#        sys.stderr.write(docid+"\n")
        count+=1
        #'''
        #extract the texts and write to new files to be aligned


        for lang in langs:
            orgfile=xmlroot+lang+"/"+year+'/'+docid+"-"+lang+".xml"
            newfile=targetdir+docid+"_"+lang+".snt"
            nf=open(newfile, 'w')
            nf.write(extract_text(orgfile, lang))
            nf.close()

  
         #'''  
  


sys.stderr.write("%d documents in all languages.\n"%(count))    

