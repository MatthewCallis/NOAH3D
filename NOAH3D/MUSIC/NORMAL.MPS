#
# NORMAL.MAP
# *.MAP files are used to control the translation of MIDI program change
# events to actual instruments to be played on the FM card.
#
# Each line takes the form: <Source> <Dest> <Filename> <Volume>
#
# <Source> specifies the patch number (from 0-127) in the MIDI file that
# you want to map.
# <Dest> specifies which patch number <Source> should be mapped to.
# <Filename> specifies the name of the instrument file to be loaded (without
# the .INS suffix). If <Filename> is '!', the current patch loaded into
# the converter (assumedly with an /INST specification) will be left in
# place.
# <Volume> specifies the volume scaling for this patch (from 1-128 - 128 is
# the loudest).
#
# Both <Filename> and <Volume> are optional.
#
# Example:
#  20 5 INST\HORN 64
#  47 5
#
# This would load the file named "INST\HORN.INS" into the converter at
# patch #5, and would map all program changes to instrument 20 in the MIDI
# file being converted to patch #5. The 64 indicates the volume scaling
# should be set to 64 (roughly half).
# The second line indicates that program changes to instrument 47 in the
# MIDI file should also use the horn, but because the instrument has
# already been loaded into the converter, and volume scaling has already
# been specified, it's not necessary to specify them again.
#
#
0	0	INST\I000	128
1	1	INST\I001	128
2	2	INST\I002	128
3	3	INST\I003	128
4	4	INST\I004	128
5	5	INST\I005	128
6	6	INST\I006	128
7	7	INST\I007	128
8	8	INST\I008	128
9	9	INST\I009	128
10      10      INST\MUSICBOX   128
11	11	INST\I011	128
12      12      INST\MARIMBA    128
13      13      INST\XYLOPHON   128
14      14      INST\TUBEBELL   128
15      15      INST\SANTUR     128
16	16	INST\I016	128
17      17      INST\ORGAN2     128
18	18	INST\I018	128
19	19	INST\I019	128
20      20      INST\REEDORGN   128
21	21	INST\I021	128
22	22	INST\I022	128
23	23	INST\I023	128
24	24	INST\I024	128
25	25	INST\I025	128
26	26	INST\I026	128
27	27	INST\I027	128
28      28      INST\MUTEDGTR   128
29	29	INST\I029	128
30	30	INST\I030	128
31	31	INST\I031	128
32      32      INST\ACOUBASS   128
33	33	INST\I033	128
34	34	INST\I034	128
35      35      INST\FTLSBASS   128
36      36      INST\SLAPBASS   128
37	37	INST\I037	128
38      38      INST\SYNBASS1   128
39      39      INST\SYNBASS2   128
40      40      INST\VIOLIN     128
41      41      INST\VIOLA      128
42      42      INST\CELLO      128
43      43      INST\CONTRABS   128
44	44	INST\I044	128
45      45      INST\PIZZSTR    128
46      46      INST\HARP       128
47      47      INST\TIMPANI    128
48      48      INST\STRINGS    128
49	49	INST\I049	128
50	50	INST\I050	128
51	51	INST\I051	128
52      52      INST\CHRL-AHS   128
53	53	INST\I053	128
54	54	INST\I054	128
55      55      INST\ORCHAHIT   128
56      56      INST\TRUMPET    128
57      57      INST\TROMBONE   128
58      58      INST\TUBA       128
59	59	INST\I059	128
60      60      INST\FRHORN     128
61      61      INST\BRASS      128
62      62      INST\SYNBRASS1  128
63      63      INST\SYNBRASS2  128
64	64	INST\I064	128
65	65	INST\I065	128
66	66	INST\I066	128
67	67	INST\I067	128
68      68      INST\OBOE       128
69      69      INST\ENGLHORN   128
70      70      INST\BASSOON    128
71      71      INST\CLARINET   128
72      72      INST\PICCOLO    128
73      73      INST\FLUTE      128
74	74	INST\I074	128
75      75      INST\PANFLUTE   128
76	76	INST\I076	128
77      77      INST\SHUKUHAC   128
78	78	INST\I078	128
79	79	INST\I079	128
80	80	INST\I080	128
81      81      INST\SAWWAVE    128
82	82	INST\I082	128
83	83	INST\I083	128
84      84      INST\CHARANG    128
85      85      INST\SOLOVOX    128
86      86      INST\5THSAW     128
87      87      INST\BS&LEAD    128
88      88      INST\FANTASIA   128
89	89	INST\I089	128
90      90      INST\POLYSYN    128
91	91	INST\I091	128
92      92      INST\BOWDGLAS   128
93      93      INST\METALPAD   128
94	94	INST\I094	128
95      95      INST\SWEEPPAD   128
96	96	INST\I096	128
97      97      INST\SOUNDTRK   128
98	98	INST\I098	128
99      99      INST\ATMOSPHR   128
100	100	INST\I100	128
101	101	INST\I101	128
102     102     INST\ECHODROP   128
103	103	INST\I103	128
104	104	INST\I104	128
105	105	INST\I105	128
106	106	INST\I106	128
107	107	INST\I107	128
108	108	INST\I108	128
109	109	INST\I109	128
110	110	INST\I110	128
111	111	INST\I111	128
112	112	INST\I112	128
113	113	INST\I113	128
114	114	INST\I114	128
115	115	INST\I115	128
116	116	INST\I116	128
117	117	INST\I117	128
118     118     INST\SYNDRUM    128
119     119     INST\REVRSCYM   128
120	120	INST\I120	128
121	121	INST\I121	128
122	122	INST\I122	128
123	123	INST\I123	128
124	124	INST\I124	128
125	125	INST\I125	128
126	126	INST\I126	128
127     127     INST\GUNSHOTS   128
162     162     INST\ACBSDRM    128
163     163     INST\BSDRUM1    128
165     165     INST\ACSNARE    128
167     167     INST\ESNARE     128
169     169     INST\HIHATCL    128
173     173     INST\HIHATOPN   128
178     178     INST\RIDECYM    128
200     200     INST\GUIRO-S    128
201     201     INST\GUIRO-L    128
