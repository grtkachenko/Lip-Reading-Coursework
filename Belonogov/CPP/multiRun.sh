

for ((alf = 50; alf < 10000; alf = alf * 2))
do
    for ((beta = 10; beta < 10000; beta = beta * 2))
    do
        res=$(./bin/CPP $alf $beta)
        echo $alf $beta $res
    done
done


