const MOCK_BOOKS = [
    {
        id: 1,
        name: "Оптимизирющие компиляторы",
        image: "https://cdn.litres.ru/pub/c/cover_415/71185981.webp",
        price: 299.99,
        author: "Константин Владимиров"
    },
    {
        id: 2,
        name: "Олимпиадное программирование",
        image: "https://cdn.litres.ru/pub/c/cover_415/44867813.webp",
        price: 199.99,
        author: "Антти Лааксонен"
    },
    {
        id: 3,
        name: "Граф Монте-Кристо. В 2 книгах. Книга 2",
        image: "https://cdn.litres.ru/pub/c/cover_415/68341772.webp",
        price: 1299.99,
        author: "Александр Дюма"
    },
    {
        id: 4,
        name: "Витя Малеев в школе и дома",
        image: "https://cdn.litres.ru/pub/c/cover_415/3140845.webp",
        price: 149.99,
        author: "Николай Носов"
    },
    {
        id: 5,
        name: "Баранкин, будь человеком!",
        image: "https://cdn.litres.ru/pub/c/cover_415/146229.webp",
        price: 499.99,
        author: "Валерий Медведев"
    },
    {
        id: 6,
        name: "Убийство в «Восточном экспрессе»",
        image: "https://cdn.litres.ru/pub/c/cover_415/18922333.webp",
        price: 79.99,
        author: "Кристи Агата"
    }
];

export const getBooks = () => {
    return new Promise((resolve) => {
        setTimeout(() => {
            resolve(MOCK_BOOKS);
        }, 100);
    });
};

export const getBook = (bookId) => {
    return new Promise((resolve) => {
        setTimeout(() => {
            resolve(MOCK_BOOKS.find(book => book.id == bookId));
        }, 100);
    });
};