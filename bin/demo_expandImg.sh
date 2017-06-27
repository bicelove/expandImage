mpwd=`pwd`
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$mpwd/../lib

####################################################
imgList='../data/test.txt'
svPath='/media/in66/文档/data/细分类/data/scene_170425_kmeans/expandImg/'
keyfilePath='../keyfile/'

rm -r $svPath
mkdir $svPath

#valgrind  --tool=memcheck  --leak-check=full --show-reachable=yes  --num-callers=30 --track-origins=yes --log-file=val.log 
./Demo_expandImg $imgList $svPath $keyfilePath
