/*

Пример для курса ФИЛП, УрФУ, 2017-2018. Веретенников А. Б.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include <string>
#include <functional>
#include <memory>
#include <algorithm>
#include <vector>
//#include <intrin.h>

using namespace std;

template <class T>
class CList
{
	struct CItem
	{
		T Value;
		shared_ptr<CItem> Tail;

		CItem()
		{

		}

		CItem(T AValue, shared_ptr<CItem> ATail) : Value(AValue), Tail(ATail)
		{

		}
	};

	shared_ptr<CItem> Head;

	CList(shared_ptr<CItem> Ptr) : Head(Ptr)
	{

	}
public:

	CList()
	{

	}

	explicit CList(T Value)
	{
		Head = make_shared<CItem>();
		Head->Value = Value;
	}

	explicit CList(initializer_list<T> L)
	{
		for (auto it = rbegin(L); it != rend(L); it++)
		{
			Head = make_shared<CItem>(*it, Head);
		}
	}

	explicit CList(T Value, CList Tail)
	{
		Head = make_shared<CItem>();
		Head->Value = Value;
		Head->Tail = Tail.Head;
	}

	CList PushFront(T Value) const
	{
		return CList(make_shared<CItem>(Value, Head));
	}

	bool ForEach(function<bool(T, size_t)> f) const
	{
		shared_ptr<CItem> Ptr = Head;
		size_t i = 0;
		while (Ptr)
		{
			if (!f(Ptr->Value, i))
				return false;
			i++;
			Ptr = Ptr->Tail;
		}

		return true;
	}

	bool ForEach(function<bool(T)> f) const
	{
		shared_ptr<CItem> Ptr = Head;
		while (Ptr)
		{
			if (!f(Ptr->Value))
				return false;
			Ptr = Ptr->Tail;
		}

		return true;
	}

	bool IsEmpty() const
	{
		return !Head;
	}

	CList Tail() const
	{
		return CList(Head->Tail);
	}

	template <class U, class F>
	CList<U> Map(F f) const
	{
		static_assert(is_convertible<F, function<U(T)> >::value, "U(T) required");

		if (IsEmpty())
		{
			return CList<U>();
		}

		return CList<U>(f(Head->Value), Tail().Map<U>(f));
	}

	template<class F>
	CList Filter1(F f) const
	{
		if (IsEmpty())
		{
			return *this;
		}

		if (f(Head->Value))
		{
			return CList(Head->Value, Tail().Filter(f));
		}
		else
		{
			return Tail().Filter(f);
		}
	}

	template<class F>
	CList Filter(F f) const
	{
		shared_ptr<CItem> Ptr = Head;

		shared_ptr<CItem> QueueStart;
		shared_ptr<CItem> QueueEnd;
		shared_ptr<CItem> New;

		while (Ptr)
		{
			if (f(Ptr->Value))
			{
				New = make_shared<CItem>();
				New->Value = Ptr->Value;
				if (QueueStart) QueueEnd->Tail = New; else QueueStart = New;
				QueueEnd = New;
			}

			Ptr = Ptr->Tail;
		}

		return CList(QueueStart);
	}

	template <class U>
	U FoldR(function<U(T, U)> f, U Start) const
	{
		if (IsEmpty())
		{
			return Start;
		}
		return f(Head->Value, Tail().FoldR(f, Start));
	}

	template <class U>
	U FoldL(function<U(U, T)> f, U Start) const
	{
		if (IsEmpty())
		{
			return Start;
		}
		U Current = f(Start, Head->Value);
		return Tail().FoldL(f, Current);
	}

	template <class U>
	U FoldL(function<U(T, T)> f) const
	{
		if (IsEmpty())
		{
			return T();
		}
		return Tail().FoldL<U>(f, Head->Value);
	}

	template <class U>
	T FoldR(function < U(T, T)> f)
	{
		if (!Head)
		{
			return T();
		}
		if (!Head->Tail)
			return Head->Value;

		return f(Head->Value, Tail().FoldR<U>(f));
	}

	CList Concat(const CList Right) const
	{
		if (IsEmpty())
		{
			return Right;
		}
		return CList(Head->Value, Tail().Concat(Right));
	}

	CList operator+(const CList& Right)
	{
		return Concat(Right);
	}

	CList Reverse() const
	{
		return FoldL<CList>([](CList A, T B) {
			return CList(B, A);
			}, CList());
	}

	string ToString() const
	{
		return Map<string>([](T Value) {
			return to_string(Value);
			}).FoldL<string>([](string A, string B) {
				return A + ", " + B;
				});
	}

	CList<CList> Group() const
	{
		if (IsEmpty())
		{
			return CList<CList>();
		}

		T Current = Head->Value;
		CList First = Filter([Current](T Value) {
			return Value == Current;
			});
		CList Second = Filter([Current](T Value) {
			return Value != Current;
			});

		if (Second.IsEmpty())
		{
			return CList<CList>(First);
		}

		return CList<CList>(First) + Second.Group();
	}

	size_t Count() const
	{
		if (IsEmpty())
		{
			return 0;
		}
		return (1 + Tail().Count());
	}

	template <class C>
	static CList FromCollection(const C& V)
	{
		shared_ptr<CItem> Item;
		for (auto it = rbegin(V); it != rend(V); it++)
		{
			Item = make_shared<CItem>(*it, Item);
		}
		return CList(Item);
	}

	vector<T> ToVector() const
	{
		vector<T> Result;
		shared_ptr<CItem> Ptr = Head;
		while (Ptr)
		{
			Result.push_back(Ptr->Value);
			Ptr = Ptr->Tail;
		}
		return Result;
	}

	CList Sort(function<bool(T, T)> F)
	{
		vector<T> Temp = ToVector();

		sort(Temp.begin(), Temp.end(), F);

		return FromCollection(Temp);
	}

	pair<CList, CList> TakeWhile(function<bool(T)> f)
	{
		CList Current = *this;
		CList Result;
		while (true)
		{
			if (Current.IsEmpty())
				break;
			if (!f(Current.Head->Value))
				break;
			Result = CList(Current.Head->Value, Result);
			Current = Current.Tail();
		}
		return make_pair(Result.Reverse(), Current);
	}

	CList<CList> Split(T Separator)
	{
		CList<CList> Result;
		CList Current = *this;

		while (true)
		{
			pair<CList, CList> P = Current.TakeWhile([Separator](T Value) {
				return Value != Separator;
				});

			//printf("%d, %s\r\n", (int)P.first.Count(), P.first.ToString().c_str());

			Result = CList<CList>(P.first, Result);
			Current = P.second;
			if (Current.IsEmpty())
			{
				break;
			}

			/*
				Remove front separator.
			*/
			Current = Current.Tail();
		}

		return Result.Reverse();
	}

	T Front() const
	{
		return Head->Value;
	}
};

class CListTest
{
public:

	void Test()
	{
		CList<int> List({ 2, 1, 0 });
		CList<int> List2(3, List);

		printf("Sum %d\r\n", List2.FoldR<int>([](int A, int B) -> int {
			return A + B;
			}, 0));
		printf("Sum %d\r\n", List2.FoldL<int>([](int A, int B) -> int {
			return A + B;
			}, 0));
		printf("Sum %d\r\n", List2.FoldL<int>([](int A, int B) -> int {
			return A + B;
			}));
		printf("Sum %d\r\n", List2.FoldR<int>([](int A, int B) -> int {
			return A + B;
			}));

		List2.Filter([](int Value) {
			return Value != 2;
			}).
			ForEach([](int Value, size_t Index) {
				printf("%d, %d\n", Value, (int)Index);
			return true; }
			);

			List2.Map<string>([](int Value) {
				return to_string(Value);
				}).ForEach([](string Value) {
					printf("%s\n", Value.c_str());
				return true;
					});

				string Str = CList<int>({ 1,2,3 })
					.Concat(CList<int>({ 4,5,6 }))
					.Map<string>([](int Value) {
					return to_string(Value);
						})
					.FoldL<string>([](string A, string B) {
							return A + B;
						}, "");

						printf("%s\r\n", Str.c_str());

						string Str1 = (CList<int>({ 1,2,3 }) + CList<int>({ 4,5,6 }))
							.Map<string>([](int Value) {
							return to_string(Value);
								})
							.FoldL<string>([](string A, string B) {
									return A + B;
								}, "");

								printf("%s\r\n", Str1.c_str());

								printf("%s\r\n", CList<int>({ 1,2,3 }).ToString().c_str());
								printf("%s\r\n", CList<int>({ 1,2,3 }).Reverse().ToString().c_str());

								printf("\r\nGroup test:\r\n");

								CList<int> GList({ 1,2,3,2,2,2,1,3,4,4,4 });
								printf("%d\r\n", (int)GList.Count());
								GList.Group().ForEach([](CList<int> Value) {
									printf("Item: %s\r\n", Value.ToString().c_str());
								return true;
									});

								printf("\r\nSort test:\r\n");
								printf("%s\r\n", CList<int>({ 1,2,10,3,4,1 }).Sort([](int A, int B) {return A < B; }).ToString().c_str());

								printf("\r\nSplit Test:\r\n");

								string Text = "to be or not to be, or or or";
								CList<char> TextList = CList<char>::FromCollection(Text);
								TextList.Split(' ').ForEach([](CList<char> Item) {
									printf("%s\r\n",
									Item.FoldL<string>([](string P, char C) {
											return P + C;
										}, "").c_str());
								return true;
									});

								/*
									Parsing file count of words functional.

									Подсчет числа слов в строке и вывод слов и их частоты со снижением частоты.
								*/
								CList<char>::FromCollection(Text).Split(' ')
									.Map<string>([](CList<char> V) {
									return V.FoldL<string>([](string P, char C) {
											return P + C;
										}, "");
										}).Group().Map<pair<string, size_t> >([](CList<string> V) {
											return make_pair(V.Front(), V.Count());
											}).Sort([](pair<string, size_t> A, pair<string, size_t> B) {
												return A.second > B.second;
												})
												.ForEach([](pair<string, size_t> P) {
													printf("%s: %d\r\n", P.first.c_str(), (int)P.second);
												return true;
													});


												/*
													Лямбда функции
													initializer_list, rbegin, rend
													functional, function<...>, чтобы разные функции вставлять в ForEach
												*/

												//TODO, tupple.
	}
};

template <class T>
class CBinominalHeap
{
	class CNode;

	typedef shared_ptr<CNode> PNode;

	typedef CList<PNode> CNodeList;

	struct CNode
	{
		size_t Rank;
		T Value;
		CNodeList Children;

		CNode(size_t ARank, T AValue, CNodeList AChildren)
		{
			Rank = ARank;
			Value = AValue;
			Children = AChildren;
		}

		CNode(size_t ARank, T AValue)
		{
			Rank = ARank;
			Value = AValue;
		}
	};

	CNodeList Heap;

	/*
		Применяется для слияния двух деревьев одинакового ранга r в дерево ранга (r+1).
		Путем добавления одного из деревьев в качестве первого элемента Children другого дерева.
		В списке Children деревья упорядочены по убыванию ранга.
	*/
	PNode Link(PNode First, PNode Second) const
	{
		if (First->Value < Second->Value)
		{
			return make_shared<CNode>(First->Rank + 1, First->Value, First->Children.PushFront(Second));
		}
		else
		{
			return make_shared<CNode>(Second->Rank + 1, Second->Value, Second->Children.PushFront(First));
		}
	}

	/**
		Ранг дерева Tree должен быть <= ранга всех деревьев в Children.
		В Children деревья упорядочены по возрастанию ранга.
	*/
	CNodeList InsTree(PNode Tree, CNodeList Children) const
	{
		if (Children.IsEmpty())
		{
			return Children.PushFront(Tree);
		}
		PNode Front = Children.Front();
		if (Tree->Rank < Front->Rank)
		{
			return Children.PushFront(Tree);
		}
		else
		{
			return InsTree(Link(Tree, Front), Children.Tail());
		}
	}

	/**
		Вставляет элемент в кучу.
		На основании элемента создаем дерево ранга 0.
		Вставляем его в кучу Children.
		Children - список деревьев, упорядоченный по возрастанию ранга.
	*/
	CNodeList Insert(T Value, CNodeList Children) const
	{
		return InsTree(make_shared<CNode>(0, Value), Children);
	}

	/**
		Оба аргумента - списки деревьев, упорядоченные по возрастанию ранга.
	*/
	CNodeList Merge(CNodeList First, CNodeList Second) const
	{
		if (First.IsEmpty())
		{
			return Second;
		}
		if (Second.IsEmpty())
		{
			return First;
		}
		PNode FirstFront = First.Front();
		PNode SecondFront = Second.Front();
		if (FirstFront->Rank < SecondFront->Rank)
		{
			return Merge(First.Tail(), Second).PushFront(FirstFront);
		}
		else
			if (SecondFront->Rank < FirstFront->Rank)
			{
				return Merge(Second.Tail(), First).PushFront(SecondFront);
			}
			else
			{
				return InsTree(Link(FirstFront, SecondFront), Merge(First.Tail(), Second.Tail()));
			}
	}

	CBinominalHeap(CNodeList AHeap) : Heap(AHeap)
	{
	}

	PNode GetMinNode() const
	{
		return Heap.FoldL<PNode>([](PNode A, PNode B) {
			if (A->Value < B->Value)
			{
				return A;
			}
			else
			{
				return B;
			}
			});
	}
public:

	CBinominalHeap()
	{

	}

	/*
		Note. Insert does not need Merge. Merge is required only for DeleteMin.
	*/
	CBinominalHeap Insert(T Value) const
	{
		return CBinominalHeap(Insert(Value, Heap));
	}

	T GetMin() const
	{
		return GetMinNode()->Value;
	}

	bool IsEmpty() const
	{
		return Heap.IsEmpty();
	}

	/*
		Для дерева узлы в списке Children упорядочены по убыванию ранга, поэтому перед Merge делаем Reverse.
		А в списке Heap деревья упорядочены по возрастанию ранга.
	*/
	CBinominalHeap DeleteMin() const
	{
		PNode MinNode = GetMinNode();
		CNodeList ExtraList = Heap.Filter([MinNode](PNode Node) {
			return Node != MinNode;
			});
		return CBinominalHeap(Merge(ExtraList, MinNode->Children.Reverse()));
	}
};



void main()
{
	printf("Binominal Heap Test\r\n");

	CBinominalHeap<int> Heap;
	Heap = Heap.Insert(7);
	Heap = Heap.Insert(3);
	Heap = Heap.Insert(1);
	for (int i = 100; i < 200; i++)
	{
		Heap = Heap.Insert(i);
	}
	Heap = Heap.Insert(10);
	printf("%d\r\n", Heap.GetMin());
	for (size_t i = 0; i < 10; i++)
	{
		Heap = Heap.DeleteMin();
		printf("%d\r\n", Heap.GetMin());
	}

	//	getchar();
}