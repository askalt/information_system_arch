import jwt
from fastapi import Form, FastAPI, Depends, HTTPException, status
from fastapi.responses import JSONResponse
from fastapi.security import OAuth2PasswordBearer, OAuth2PasswordRequestForm
from pydantic import BaseModel
from typing import List, Dict, Optional
from fastapi.middleware.cors import CORSMiddleware

from passlib.context import CryptContext
from datetime import datetime, timedelta

app = FastAPI()

# CORS
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

books_db: Dict[int, Dict] = {
    1: {
        "id": 1,
        "name": "Оптимизирющие компиляторы",
        "image": "https://cdn.litres.ru/pub/c/cover_415/71185981.webp",
        "price": 299.99,
        "author": "Константин Владимиров"
    },
    2: {
        "id": 2,
        "name": "Олимпиадное программирование",
        "image": "https://cdn.litres.ru/pub/c/cover_415/44867813.webp",
        "price": 199.99,
        "author": "Антти Лааксонен"
    },
    3: {
        "id": 3,
        "name": "Граф Монте-Кристо. В 2 книгах. Книга 2",
        "image": "https://cdn.litres.ru/pub/c/cover_415/68341772.webp",
        "price": 1299.99,
        "author": "Александр Дюма"
    },
    4: {
        "id": 4,
        "name": "Витя Малеев в школе и дома",
        "image": "https://cdn.litres.ru/pub/c/cover_415/3140845.webp",
        "price": 149.99,
        "author": "Николай Носов"
    },
    5: {
        "id": 5,
        "name": "Баранкин, будь человеком!",
        "image": "https://cdn.litres.ru/pub/c/cover_415/146229.webp",
        "price": 499.99,
        "author": "Валерий Медведев"
    },
    6: {
        "id": 6,
        "name": "Убийство в «Восточном экспрессе»",
        "image": "https://cdn.litres.ru/pub/c/cover_415/18922333.webp",
        "price": 79.99,
        "author": "Кристи Агата"
    }
}

user_info_db: Dict[int, Dict] = {
    1: {"id": 1,
    	"name": "Astronomax",
    	"image": "https://i.namu.wiki/i/X1NSMaWzWMiLwz7lTOfl65kbteTqO7DXAVscpSlU3FD0yRv35Jj2UYxdUJnfIz6TfDoRPwFfuGPp5LDCowwjxQ.webp"},
    2: {"id": 2,
    	"name": "Arrias",
    	"image": "https://i.namu.wiki/i/X1NSMaWzWMiLwz7lTOfl65kbteTqO7DXAVscpSlU3FD0yRv35Jj2UYxdUJnfIz6TfDoRPwFfuGPp5LDCowwjxQ.webp"},
}

reviews_db: Dict[int, List[Dict]] = {
    1: [
        {"id": 1, "rating": 5, "text": "Отличная книга! Рекомендую всем.", "bookId": 1, "userId": 1},
        {"id": 2, "rating": 4, "text": "Хорошая книга, но местами скучная.", "bookId": 1, "userId": 2},
    ],
    2: [
        {"id": 3, "rating": 5, "text": "Отличная книга! Рекомендую всем.", "bookId": 2, "userId": 1},
        {"id": 4, "rating": 4, "text": "Хорошая книга, но местами скучная.", "bookId": 2, "userId": 2},
    ],
}

review_id = 5

class Review(BaseModel):
    id: int = None
    rating: int
    text: str
    userId: int

class Book(BaseModel):
    id: int
    name: str
    image: str
    price: float
    author: str

class UserInfo(BaseModel):
    id: int = None
    name: str
    image: str = ""

class UserInDB(UserInfo):
    email: str
    password: str

@app.get("/getBooks", response_model=List[Book])
async def get_books():
    return list(books_db.values())

@app.get("/getBook/{book_id}", response_model=Book)
async def get_book(book_id: int):
    book = books_db.get(book_id)
    if not book:
        raise HTTPException(status_code=404, detail="Book not found")
    return book

@app.get("/getUsers", response_model=List[UserInfo])
async def get_users():
    return list(user_info_db.values())

@app.get("/getUser/{user_id}", response_model=UserInfo)
async def get_user(user_id: int):
    user = user_info_db.get(user_id)
    if not user:
        raise HTTPException(status_code=404, detail="User not found")
    return user

@app.get("/getReviews/{book_id}", response_model=List[Review])
async def get_reviews(book_id: int):
    if book_id not in reviews_db:
        return []

    return reviews_db[book_id]

user_db = {
    "a": {"id": 1,
    	"name": "Astronomax",
        "email": "a",
    	"image": "https://i.namu.wiki/i/X1NSMaWzWMiLwz7lTOfl65kbteTqO7DXAVscpSlU3FD0yRv35Jj2UYxdUJnfIz6TfDoRPwFfuGPp5LDCowwjxQ.webp",
        "password": "a"},
    "b": {"id": 2,
    	"name": "Arrias",
        "email": "b",
    	"image": "https://i.namu.wiki/i/X1NSMaWzWMiLwz7lTOfl65kbteTqO7DXAVscpSlU3FD0yRv35Jj2UYxdUJnfIz6TfDoRPwFfuGPp5LDCowwjxQ.webp",
        "password": "a"},
}

user_id = 3

SECRET_KEY = "your_secret_key"
ALGORITHM = "HS256"
ACCESS_TOKEN_EXPIRE_MINUTES = 1
REFRESH_TOKEN_EXPIRE_DAYS = 7

pwd_context = CryptContext(schemes=["bcrypt"], deprecated="auto")

oauth2_scheme = OAuth2PasswordBearer(tokenUrl="token")

class Token(BaseModel):
    access_token: str
    refresh_token: str

class TokenRefresh(BaseModel):
    refresh_token: str

def verify_password(plain_password, hashed_password):
    return pwd_context.verify(plain_password, hashed_password)

def get_password_hash(password):
    return pwd_context.hash(password)

def create_access_token(data: dict, expires_delta: timedelta = timedelta(minutes=ACCESS_TOKEN_EXPIRE_MINUTES)):
    to_encode = data.copy()
    expire = datetime.utcnow() + expires_delta
    to_encode.update({"exp": expire})
    encoded_jwt = jwt.encode(to_encode, SECRET_KEY, algorithm=ALGORITHM)
    return encoded_jwt

def create_refresh_token(data: dict, expires_delta: timedelta = timedelta(days=REFRESH_TOKEN_EXPIRE_DAYS)):
    to_encode = data.copy()
    expire = datetime.utcnow() + expires_delta
    to_encode.update({"exp": expire})
    encoded_jwt = jwt.encode(to_encode, SECRET_KEY, algorithm=ALGORITHM)
    return encoded_jwt

def verify_token(token: str = Depends(oauth2_scheme)):
    try:
        payload = jwt.decode(token, SECRET_KEY, algorithms=[ALGORITHM])
        user_id = payload.get("user_id")
        if user_id is None:
            raise HTTPException(status_code=401, detail="Invalid token")
        return user_id
    except jwt.PyJWTError:
        raise HTTPException(status_code=status.HTTP_401_UNAUTHORIZED, detail="Could not validate credentials")

@app.post("/submitReview/{book_id}", response_model=Review)
async def submit_review(book_id: int, review: Review, user_id: int = Depends(verify_token)):
    if book_id not in books_db:
        raise HTTPException(status_code=404, detail="Book not found")

    if book_id not in reviews_db:
        reviews_db[book_id] = []

    global review_id
    review = review.dict()
    review["id"] = review_id
    review["userId"] = user_id
    reviews_db[book_id].append(review)
    review_id += 1
    return review

@app.post("/removeReview/{review_id}", response_model=Review)
async def remove_review(review_id: int, user_id: int = Depends(verify_token)):
    review_to_delete = None
    for book_reviews in reviews_db.values():
        for review in book_reviews:
            if review["id"] == review_id:
                review_to_delete = review
                break
        if review_to_delete:
            break
    if review_to_delete is None:
        raise HTTPException(status_code=404, detail="Review not found")
    if review_to_delete["userId"] != user_id:
        raise HTTPException(status_code=403, detail="You are not authorized to delete this review")
    for book_reviews in reviews_db.values():
        if review_to_delete in book_reviews:
            book_reviews.remove(review_to_delete)
            break
    return review_to_delete

@app.post("/token")
async def login_for_access_token(
    email: str = Form(...), 
    password: str = Form(...)
):
    user = user_db.get(email)
    if not user or not verify_password(password, user["password"]):
        raise HTTPException(status_code=status.HTTP_401_UNAUTHORIZED, detail="Incorrect email or password")

    access_token = create_access_token(data={"user_id": user["id"]})
    refresh_token = create_refresh_token(data={"user_id": user["id"]})

    response = JSONResponse(content={"access_token": access_token, "refresh_token": refresh_token})
    #response.set_cookie(key="refresh_token", value=refresh_token, httponly=True, secure=True)
    return response

@app.post("/refresh", response_model=Token)
async def refresh_access_token(token_data: TokenRefresh):
    try:
        payload = jwt.decode(token_data.refresh_token, SECRET_KEY, algorithms=[ALGORITHM])
        user_id = payload.get("user_id")
        
        if user_id is None:
            raise HTTPException(status_code=403, detail="Invalid refresh token")

        new_access_token = create_access_token(data={"user_id": user_id})
        return {"access_token": new_access_token, "refresh_token": token_data.refresh_token}

    except jwt.PyJWTError:
        raise HTTPException(status_code=403, detail="Invalid refresh token")

@app.post("/register")
async def register(user: UserInDB):
    if user.email in user_db:
        raise HTTPException(status_code=status.HTTP_400_BAD_REQUEST, detail="email already registered")

    global user_id
    user_db[user.email] = {
        "id": user_id,
        "name": user.name,
        "image": user.image,
        "email": user.email,
        "password": get_password_hash(user.password)
    }
    user_info_db[user_id] = {
        "id": user_id,
        "name": user.name,
        "image": user.image,
    }
    user_id += 1
    return {"message": "User registered successfully"}


class CartItem(BaseModel):
    book_id: int
    quantity: int = None

cart_db: Dict[int, List[CartItem]] = {}

@app.get("/cart/{user_id}", response_model=List[CartItem])
async def get_cart(user_id: int = Depends(verify_token)):
    if user_id not in user_info_db:
        raise HTTPException(status_code=status.HTTP_400_BAD_REQUEST, detail="Unknown user")
    if user_id not in cart_db:
        raise HTTPException(status_code=404, detail="Cart not found")
    return cart_db[user_id]

@app.post("/cart/{user_id}/add", response_model=CartItem)
async def add_to_cart(cart_item: CartItem, user_id: int = Depends(verify_token)):
    if user_id not in user_info_db:
        raise HTTPException(status_code=status.HTTP_400_BAD_REQUEST, detail="Unknown user")
    if user_id not in cart_db:
        cart_db[user_id] = []

    for item in cart_db[user_id]:
        if item.book_id == cart_item.book_id:
            item.quantity += 1
            return item

    new_item = CartItem(book_id=cart_item.book_id, quantity=1)
    cart_db[user_id].append(new_item)
    return new_item

@app.delete("/cart/{book_id}/remove", response_model=CartItem)
async def remove_from_cart(book_id: int, user_id: int = Depends(verify_token)):
    if user_id not in user_info_db:
        raise HTTPException(status_code=status.HTTP_400_BAD_REQUEST, detail="Unknown user")
    if user_id not in cart_db:
        raise HTTPException(status_code=404, detail="Cart not found")
    
    cart_items = cart_db[user_id]
    for item in cart_items:
        if item.book_id == book_id:
            cart_items.remove(item)
            return item
    
    raise HTTPException(status_code=404, detail="Book not found in cart")

@app.put("/cart/{book_id}/increase", response_model=CartItem)
async def decrease_quantity(book_id: int, user_id: int = Depends(verify_token)):
    if user_id not in user_info_db:
        raise HTTPException(status_code=status.HTTP_400_BAD_REQUEST, detail="Unknown user")
    if user_id not in cart_db:
        raise HTTPException(status_code=404, detail="Cart not found")

    cart_items = cart_db[user_id]
    for item in cart_items:
        if item.book_id == book_id:
            item.quantity += 1
            return item
    
    raise HTTPException(status_code=404, detail="Book not found in cart")

@app.put("/cart/{book_id}/decrease", response_model=CartItem)
async def decrease_quantity(book_id: int, user_id: int = Depends(verify_token)):
    if user_id not in user_info_db:
        raise HTTPException(status_code=status.HTTP_400_BAD_REQUEST, detail="Unknown user")
    if user_id not in cart_db:
        raise HTTPException(status_code=404, detail="Cart not found")

    cart_items = cart_db[user_id]
    for item in cart_items:
        if item.book_id == book_id:
            if item.quantity > 1:
                item.quantity -= 1
                return item
            else:
                cart_items.remove(item)
                return item
    
    raise HTTPException(status_code=404, detail="Book not found in cart")
