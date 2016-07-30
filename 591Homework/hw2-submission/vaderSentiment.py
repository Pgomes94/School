import os, math, re, sys, fnmatch, string 

def make_lex_dict(f):
    return dict(map(lambda (w, m): (w, float(m)), [wmsr.strip().split('\t')[0:2] for wmsr in open(f) ]))

def sentiment(wordsAndEmoticons, word_valence_dict):
    """
    Returns a float for sentiment strength based on the input text.
    Positive values are positive valence, negative value are negative valence.
    """
    
    # remove stopwords from [wordsAndEmoticons]
    #stopwords = [str(word).strip() for word in open('stopwords.txt')]
    #for word in wordsAndEmoticons:
    #    if word in stopwords:
    #        wordsAndEmoticons.remove(word)
    
    # check for negation
    negate = ["aint", "arent", "cannot", "cant", "couldnt", "darent", "didnt", "doesnt",
              "ain't", "aren't", "can't", "couldn't", "daren't", "didn't", "doesn't",
              "dont", "hadnt", "hasnt", "havent", "isnt", "mightnt", "mustnt", "neither",
              "don't", "hadn't", "hasn't", "haven't", "isn't", "mightn't", "mustn't",
              "neednt", "needn't", "never", "none", "nope", "nor", "not", "nothing", "nowhere", 
              "oughtnt", "shant", "shouldnt", "uhuh", "wasnt", "werent",
              "oughtn't", "shan't", "shouldn't", "uh-uh", "wasn't", "weren't",  
              "without", "wont", "wouldnt", "won't", "wouldn't", "rarely", "seldom", "despite"]
    def negated(list, nWords=[], includeNT=True):
        nWords.extend(negate)
        for word in nWords:
            if word in list:
                return True
        if includeNT:
            for word in list:
                if "n't" in word:
                    return True
        if "least" in list:
            i = list.index("least")
            if i > 0 and list[i-1] != "at":
                return True
        return False
        
    def normalize(score, alpha=15):
        # normalize the score to be between -1 and 1 using an alpha that approximates the max expected value 
        normScore = score/math.sqrt( ((score*score) + alpha) )
        return normScore
    
    b_incr = 0.293 #(empirically derived mean sentiment intensity rating increase for booster words)
    b_decr = -0.293
    # booster/dampener 'intensifiers' or 'degree adverbs' http://en.wiktionary.org/wiki/Category:English_degree_adverbs
    booster_dict = {"absolutely": b_incr, "amazingly": b_incr, "awfully": b_incr, "completely": b_incr, "considerably": b_incr, 
                    "decidedly": b_incr, "deeply": b_incr, "effing": b_incr, "enormously": b_incr, 
                    "entirely": b_incr, "especially": b_incr, "exceptionally": b_incr, "extremely": b_incr,
                    "fabulously": b_incr, "flipping": b_incr, "flippin": b_incr, 
                    "fricking": b_incr, "frickin": b_incr, "frigging": b_incr, "friggin": b_incr, "fully": b_incr, "fucking": b_incr, 
                    "greatly": b_incr, "hella": b_incr, "highly": b_incr, "hugely": b_incr, "incredibly": b_incr, 
                    "intensely": b_incr, "majorly": b_incr, "more": b_incr, "most": b_incr, "particularly": b_incr, 
                    "purely": b_incr, "quite": b_incr, "really": b_incr, "remarkably": b_incr, 
                    "so": b_incr,  "substantially": b_incr, 
                    "thoroughly": b_incr, "totally": b_incr, "tremendously": b_incr, 
                    "uber": b_incr, "unbelievably": b_incr, "unusually": b_incr, "utterly": b_incr, 
                    "very": b_incr, 
                    
                    "almost": b_decr, "barely": b_decr, "hardly": b_decr, "just enough": b_decr, 
                    "kind of": b_decr, "kinda": b_decr, "kindof": b_decr, "kind-of": b_decr,
                    "less": b_decr, "little": b_decr, "marginally": b_decr, "occasionally": b_decr, "partly": b_decr, 
                    "scarcely": b_decr, "slightly": b_decr, "somewhat": b_decr, 
                    "sort of": b_decr, "sorta": b_decr, "sortof": b_decr, "sort-of": b_decr}
    sentiments = []
    for item in wordsAndEmoticons:
        v = 0
        i = wordsAndEmoticons.index(item)
        if (i < len(wordsAndEmoticons)-1 and str(item) == "kind" and \
           str(wordsAndEmoticons[i+1]) == "of") or str(item) in booster_dict:
            sentiments.append(v)
            continue
        if  item in word_valence_dict:
            #get the sentiment valence
            v = float(word_valence_dict[item])
            
            #check if the preceding words increase, decrease, or negate/nullify the valence
            def scalar_inc_dec(word, valence):
                scalar = 0.0
                if word in booster_dict:
                    scalar = booster_dict[word]
                    if valence < 0: scalar *= -1
                return scalar
            n_scalar = -0.74
            if i > 0 and str(wordsAndEmoticons[i-1]) not in word_valence_dict:
                s1 = scalar_inc_dec(wordsAndEmoticons[i-1], v)
                v = v+s1
                if negated([wordsAndEmoticons[i-1]]): v = v*n_scalar
            if i > 1 and str(wordsAndEmoticons[i-2]) not in word_valence_dict:
                s2 = scalar_inc_dec(wordsAndEmoticons[i-2], v)
                if s2 != 0: s2 = s2*0.95
                v = v+s2
                # check for special use of 'never' as valence modifier instead of negation
                if wordsAndEmoticons[i-2] == "never" and (wordsAndEmoticons[i-1] == "so" or wordsAndEmoticons[i-1] == "this"): 
                    v = v*1.5                    
                # otherwise, check for negation/nullification
                elif negated([wordsAndEmoticons[i-2]]): v = v*n_scalar
            if i > 2 and str(wordsAndEmoticons[i-3]) not in word_valence_dict:
                s3 = scalar_inc_dec(wordsAndEmoticons[i-3], v)
                if s3 != 0: s3 = s3*0.9
                v = v+s3
                # check for special use of 'never' as valence modifier instead of negation
                if wordsAndEmoticons[i-3] == "never" and \
                   (wordsAndEmoticons[i-2] == "so" or wordsAndEmoticons[i-2] == "this") or \
                   (wordsAndEmoticons[i-1] == "so" or wordsAndEmoticons[i-1] == "this"):
                    v = v*1.25
                # otherwise, check for negation/nullification
                elif negated([wordsAndEmoticons[i-3]]): v = v*n_scalar
                
                # check for special case idioms using a sentiment-laden keyword known to SAGE
                special_case_idioms = {"the shit": 3, "the bomb": 3, "bad ass": 1.5, "yeah right": -2, 
                                       "cut the mustard": 2, "kiss of death": -1.5, "hand to mouth": -2}
                # future work: consider other sentiment-laden idioms
                #other_idioms = {"back handed": -2, "blow smoke": -2, "blowing smoke": -2, "upper hand": 1, "break a leg": 2, 
                #                "cooking with gas": 2, "in the black": 2, "in the red": -2, "on the ball": 2,"under the weather": -2}
                onezero = "{} {}".format(str(wordsAndEmoticons[i-1]), str(wordsAndEmoticons[i]))
                twoonezero = "{} {} {}".format(str(wordsAndEmoticons[i-2]), str(wordsAndEmoticons[i-1]), str(wordsAndEmoticons[i]))
                twoone = "{} {}".format(str(wordsAndEmoticons[i-2]), str(wordsAndEmoticons[i-1]))
                threetwoone = "{} {} {}".format(str(wordsAndEmoticons[i-3]), str(wordsAndEmoticons[i-2]), str(wordsAndEmoticons[i-1]))
                threetwo = "{} {}".format(str(wordsAndEmoticons[i-3]), str(wordsAndEmoticons[i-2]))                    
                if onezero in special_case_idioms: v = special_case_idioms[onezero]
                elif twoonezero in special_case_idioms: v = special_case_idioms[twoonezero]
                elif twoone in special_case_idioms: v = special_case_idioms[twoone]
                elif threetwoone in special_case_idioms: v = special_case_idioms[threetwoone]
                elif threetwo in special_case_idioms: v = special_case_idioms[threetwo]
                if len(wordsAndEmoticons)-1 > i:
                    zeroone = "{} {}".format(str(wordsAndEmoticons[i]), str(wordsAndEmoticons[i+1]))
                    if zeroone in special_case_idioms: v = special_case_idioms[zeroone]
                if len(wordsAndEmoticons)-1 > i+1:
                    zeroonetwo = "{} {}".format(str(wordsAndEmoticons[i]), str(wordsAndEmoticons[i+1]), str(wordsAndEmoticons[i+2]))
                    if zeroonetwo in special_case_idioms: v = special_case_idioms[zeroonetwo]
                
                # check for booster/dampener bi-grams such as 'sort of' or 'kind of'
                if threetwo in booster_dict or twoone in booster_dict:
                    v = v+b_decr
            
            # check for negation case using "least"
            if i > 1 and str(wordsAndEmoticons[i-1]) not in word_valence_dict \
                and str(wordsAndEmoticons[i-1]) == "least":
                if (str(wordsAndEmoticons[i-2]) != "at" and str(wordsAndEmoticons[i-2]) != "very"):
                    v = v*n_scalar
            elif i > 0 and str(wordsAndEmoticons[i-1]) not in word_valence_dict \
                and str(wordsAndEmoticons[i-1]) == "least":
                v = v*n_scalar
        sentiments.append(v) 
            
    # check for modification in sentiment due to contrastive conjunction 'but'
    if 'but' in wordsAndEmoticons:
        try: bi = wordsAndEmoticons.index('but')
        except: bi = wordsAndEmoticons.index('BUT')
        for s in sentiments:
            si = sentiments.index(s)
            if si < bi: 
                sentiments.pop(si)
                sentiments.insert(si, s*0.5)
            elif si > bi: 
                sentiments.pop(si)
                sentiments.insert(si, s*1.5) 
                
    if sentiments:                      
        sum_s = float(sum(sentiments))
        #print sentiments, sum_s
        compound = normalize(sum_s)
    else:
        compound = 0.0
    return round(compound,4)
