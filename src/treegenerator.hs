import Data.List
import System.Environment

data Intree a = Intree a [Intree a]

instance (Show a) => Show (Intree a) where
    show (Intree v c) = "(" ++ show v ++ concat [show x | x <- c] ++ ")"

instance Eq (Intree a) where
    (Intree x c) == (Intree y d) = if length c /= length d then False else all (\(t1, t2) -> t1 == t2) $ zip c d

instance Ord (Intree a) where
    (Intree x c) < (Intree y d) = let
        lc = length c
        ld = length d
        in if lc >= ld then False else all (\(t1, t2) -> t1 < t2) $ zip (sort c) (sort d)
    a <= b = a < b || a==b
    a > b = not $ a <= b
    a >= b = not $ a < b

treetolist :: (Intree Int) -> [Int] 
treetolist it = let
    ttl (Intree val chi) me pre = first ++ concat [ttl c m m | (c, m) <- zip chi (iterate (1+) me)]
        where first = (if pre < 0 then [] else [pre])
    in ttl it 0 (-1)

countleaves :: (Intree a) -> Int
countleaves (Intree _ []) = 1
countleaves (Intree _ c) = sum [countleaves child | child <- c]

partitions :: Int -> Int -> [[Int]]
partitions n r = let
    partis n 0 m = []
    partis 0 1 m = [[]]
    partis n 1 m = [[n]]
    partis n r m = [h:t | h <- [m..n-1], t <- partis (n-h) (r-1) h, h <= quot n 2]
    in partis n r 1

combinations :: [[[a]]] -> [[a]]
combinations [] = [[]]
combinations (x:xs) = [h++t | h <- x, t <- combinations xs]

multisets :: [a] -> Int -> [[a]]
multisets _ 0 = [[]]
multisets [] _ = []
multisets l 1 = [[c] | c <- l]
multisets (x:xs) n = [x:t | t <- multisets (x:xs) (n-1)] ++ [t | t <- multisets xs n]

generateintrees :: Int -> [Intree Int]
generateintrees 1 = [Intree 0 []]
generateintrees n = let 
    sortedintreecombos :: Int -> Int -> [[Intree Int]]
    sortedintreecombos n x = let
        trees = generateintrees n 
        in multisets trees x
    intreecombos :: [Int] -> [[Intree Int]]
    intreecombos part = let 
        stuff = [sortedintreecombos (head p) (length p) | p <- group part]
        in combinations stuff
    in [Intree 0 c | r <- [1..n-1], p <- partitions (n-1) r, c <- intreecombos p]

printlist :: (Show a) => [a] -> IO ()
printlist [] = putStr "\n"
printlist [x] = do
    putStr $ show x
    putStr "\n"
printlist (x:xs) = do
    putStr $ show x
    putStr " "
    printlist xs

printtrees n minleaves = mapM_ printlist [treetolist t | t <- filter (\x -> minleaves <= countleaves x) $ generateintrees n]

main = do
    args <- getArgs
    printtrees (read $ args!!0) (read $ args!!1)
    print args
