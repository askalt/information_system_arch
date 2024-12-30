const MOCK_USERS = [
    {
        id: 1,
        name: "Astronomax",
        image: "https://i.namu.wiki/i/X1NSMaWzWMiLwz7lTOfl65kbteTqO7DXAVscpSlU3FD0yRv35Jj2UYxdUJnfIz6TfDoRPwFfuGPp5LDCowwjxQ.webp",
    },
    {
        id: 2,
        name: "Arrias",
        image: "https://i.namu.wiki/i/X1NSMaWzWMiLwz7lTOfl65kbteTqO7DXAVscpSlU3FD0yRv35Jj2UYxdUJnfIz6TfDoRPwFfuGPp5LDCowwjxQ.webp",
    },
];

export const getUsers = () => {
    return new Promise((resolve) => {
        setTimeout(() => {
            resolve(MOCK_USERS);
        }, 100);
    });
};

export const getUser = (userId) => {
    return new Promise((resolve) => {
        setTimeout(() => {
            resolve(MOCK_USERS.find(user => user.id == userId));
        }, 100);
    });
};